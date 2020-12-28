#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "Listener.h"

namespace gogo {
    namespace {

// Returns a vector of all of the components of the provided path.
// "/foo/bar"  ->  [ "/", "foo", "bar" ]
        std::vector<std::string> GetPathComponents(const FilePath &path) {
            std::vector<std::string> ret_val;
            FilePath current = path;
            FilePath base;

            // Capture path components.
            while (current != current.parent_path()) {
                base = current.leaf();
                if (!base.empty() && base == L".") {
                    current = current.parent_path();
                    continue;
                }
                ret_val.push_back(base.leaf().native());
                current = current.parent_path();
            }

            return std::vector<std::string>(ret_val.rbegin(), ret_val.rend());
        }

        class FilePathWatcherImpl;

        class InotifyReader;

// Singleton to manage all inotify watches.
        class InotifyReader : private boost::noncopyable {
         public:
            using Watch = int;  // Watch descriptor used by AddWatch() and RemoveWatch().
            static constexpr Watch INVALID_WATCH = -1;

         public:
            // Watch directory |path| for changes. |watcher| will be notified on each
            // change. Returns |INVALID_WATCH| on failure.
            Watch AddWatch(const FilePath &path, FilePathWatcherImpl *watcher);

            // Remove |watch| if it's valid.
            void RemoveWatch(Watch watch, FilePathWatcherImpl *watcher);

            // Callback for InotifyReaderTask.
            void OnInotifyEvent(const inotify_event *event);

            static InotifyReader &Get() {
                static InotifyReader inotify_reader;
                return inotify_reader;
            }

            // Indicates the InotifyReader is in invalid state if inotify_init or create child thread failed.
            bool IsValid() const { return valid_; }

            std::recursive_mutex &GetMutex() { return mutex_; }

         private:
            InotifyReader();

            ~InotifyReader() = default;

            // Returns true on successful thread creation.
            bool StartThread();

            // Thread entry point
            void ThreadMain();

         private:
            // We keep track of which delegates want to be notified on which watches.
            std::unordered_map<Watch, std::unordered_set<FilePathWatcherImpl *>> watchers_;

            // File descriptor returned by inotify_init.
            const int inotify_fd_;

            // Flag set to true when startup was successful.
            bool valid_ = false;

            // Lock to protect InotifyReader and FilePathWatcherImpl shared content.
            // FilePathWatcher::Watch and Cancel are invoked from the main thread and thus
            // we must make sure that shared data between the main thread and select thread are synchronized.
            // std::recursive_mutex is used since it is possible to call lock in recursive manner.
            std::recursive_mutex mutex_;
        };

        class FilePathWatcherImpl : public FilePathWatcher::FilePathWatcherDelegate {
         public:
            FilePathWatcherImpl() = default;

            ~FilePathWatcherImpl() override = default;

            // Called for each event coming from the watch. |fired_watch| identifies the
            // watch that fired, |child| indicates what has changed, and is relative to
            // the currently watched path for |fired_watch|.
            //
            // |created| is true if the object appears.
            // |deleted| is true if the object disappears.
            void OnFilePathChanged(InotifyReader::Watch fired_watch,
                                   const std::string &child,
                                   bool created,
                                   bool deleted,
                                   bool is_dir);

         private:
            FilePath main_path_;

            // Inotify watches are installed for all directory components of |target_|.
            // A WatchEntry instance holds:
            // - |watch|: the watch descriptor for a component.
            // - |subdir|: the subdirectory that identifies the next component.
            //   - For the last component, there is no next component, so it is empty.
            // - |linkname|: the target of the symlink.
            //   - Only if the target being watched is a symbolic link.
            // - |full_path|: full path from root to the current |subdir|, |subdir| not included to the |full_path|.
            struct WatchEntry {
                explicit WatchEntry(const std::string &dirname) :
                        watch(InotifyReader::INVALID_WATCH),
                        subdir(dirname) {}

                InotifyReader::Watch watch;
                std::string subdir;
                std::string linkname;
                gogo::FilePath full_path;
            };

         private:
            // Start watching |path| for changes and notify |delegate| on each change.
            // Returns true if watch for |path| has been added successfully.
            bool Watch(const FilePath &path,
                       const FilePathWatcher::Callback &callback) override;

            // Cancel the watch. This unregisters the instance with InotifyReader.
            void Cancel() override;

            // Reconfigure to watch for the most specific parent directory of |target_|
            // that exists.
            void UpdateWatches();

            void RecurciveUpdateWatches(boost::filesystem::path path, WatchEntry watch_entry);

            // |path| is a symlink to a non-existent target. Attempt to add a watch to
            // the link target's parent directory. Update |watch_entry| on success.
            void AddWatchForBrokenSymlink(const FilePath &path, WatchEntry *watch_entry);

            bool HasValidWatchVector() const;

         private:
            // Callback to notify upon changes.
            FilePathWatcher::Callback callback_;

            // The file or directory we're supposed to watch.
            FilePath target_;

            // TODO(i.saneev): Unsupported yet
            bool recursive_ = false;

            // The vector of watches and next component names for all path components,
            // starting at the root directory. The last entry corresponds to the watch for
            // |target_| and always stores an empty next component name in |subdir|.
            std::vector<WatchEntry> watches_;

            // The number of inotify watches currently associated with this instance.
            int number_of_inotify_watches_ = 0;
        };

        void FilePathWatcherImpl::OnFilePathChanged(InotifyReader::Watch fired_watch,
                                                    const std::string &child,
                                                    bool created,
                                                    bool deleted,
                                                    bool is_dir) {
            assert(!watches_.empty());
            assert(HasValidWatchVector());
            if (!HasValidWatchVector())
                return;

            // Used below to avoid multiple updates.
            bool updated = false;

            // Find the entry in |watches_| that corresponds to |fired_watch|.
            for (size_t i = 0; i < watches_.size(); ++i) {
                const WatchEntry &watch_entry = watches_[i];
                if (fired_watch != watch_entry.watch)
                    continue;

                if (is_dir && created && watch_entry.full_path.string().rfind(main_path_.string()) != std::string::npos) {
                    watches_.pop_back();
                    auto path = watch_entry.full_path / child;
                    auto index_of_file_begin = path.string().rfind('/');
                    auto ii = WatchEntry(path.string().substr(index_of_file_begin + 1));
                    RecurciveUpdateWatches(path, ii);
                    watches_.push_back(WatchEntry(std::string()));
                }

                // Check whether a path component of |target_| changed.
                bool change_on_target_path =
                        child.empty() ||
                        (child == watch_entry.linkname) ||
                        (child == watch_entry.subdir);

                // Check if the change references |target_| or a direct child of |target_|.
                bool target_changed = false;
                if (!is_dir) {
                    // The fired watch is for a WatchEntry without a subdir. Thus for a given
                    // |target_| = "/path/to/foo", this is for "foo". Here, check either:
                    // - the target has no symlink: it is the target and it changed.
                    // - the target has a symlink, and it matches |child|.
                    target_changed = (watch_entry.linkname.empty() ||
                                      child == watch_entry.linkname);
                } else {
                    // The fired watch is for a WatchEntry with a subdir. Thus for a given
                    // |target_| = "/path/to/foo", this is for {"/", "/path", "/path/to"}.
                    // So we can safely access the next WatchEntry since we have not reached
                    // the end yet. Check |watch_entry| is for "/path/to", i.e. the next
                    // element is "foo".
                    const bool next_watch_may_be_for_target = watches_[i + 1].subdir.empty();
                    if (next_watch_may_be_for_target) {
                        // The current |watch_entry| is for "/path/to", so check if the |child|
                        // that changed is "foo".
                        target_changed = watch_entry.subdir == child;
                    } else {
                        // The current |watch_entry| is not for "/path/to", so the next entry
                        // cannot be "foo". Thus |target_| has not changed.
                        if (watch_entry.full_path.string().rfind(main_path_.string()) != std::string::npos)
                            target_changed = true;
                        else
                            target_changed = false;
                    }
                }

                // Update watches if a directory component of the |target_| path (dis)appears.
//                if (change_on_target_path && (created || deleted) && !updated) {
//                    UpdateWatches();
//                    updated = true;
//                }

                // Report the following events:
                //  - The target or a direct child of the target got changed (in case the
                //    watched path refers to a directory).
                //  - One of the parent directories got moved or deleted, since the target
                //    disappears in this case.
                //  - One of the parent directories appears. The event corresponding to
                //    the target appearing might have been missed in this case, so recheck.
                boost::system::error_code ec;
                if (target_changed ||
                    (change_on_target_path && deleted) ||
                    (change_on_target_path && created && boost::filesystem::exists(target_, ec))) {
                    FilePathWatcher::Event event = FilePathWatcher::Event::NO_EVENT;
                    if (is_dir)
                        event = FilePathWatcher::Event::NO_EVENT;
                    else if (deleted)
                        event = FilePathWatcher::Event::DELETED;
                    else if (created)
                        event = FilePathWatcher::Event::CREATED;
                    callback_(watch_entry.full_path / child, /*error=*/false, event);
                    return;
                }
            }
        }

        bool FilePathWatcherImpl::Watch(const FilePath &path,
                                        const FilePathWatcher::Callback &callback) {
            if (!InotifyReader::Get().IsValid() || path.empty())
                return false;

            assert(target_.empty());

            std::unique_lock<std::recursive_mutex> lock(InotifyReader::Get().GetMutex());

            main_path_ = path;
            callback_ = callback;
            target_ = path.is_relative() ? boost::filesystem::absolute(path) : path;

            const std::vector<std::string> comps = GetPathComponents(target_);
            assert(!comps.empty());
            for (size_t i = 1; i < comps.size(); ++i) {
                if (comps[i].empty())
                    continue;
                watches_.push_back(WatchEntry(comps[i]));
            }
            // For the last component, there is no next component and subdir is empty and
            // we must add an empty |WatchEntry| explicitly.
            watches_.push_back(WatchEntry(std::string()));
            UpdateWatches();
            return true;
        }

        void FilePathWatcherImpl::Cancel() {
            if (!callback_) {
                // Watch() was never called.
                SetCancelled();
                return;
            }

            std::unique_lock<std::recursive_mutex> lock(InotifyReader::Get().GetMutex());

            assert(!GetCancelled());

            SetCancelled();
            callback_ = {};

            for (const auto &watch : watches_)
                InotifyReader::Get().RemoveWatch(watch.watch, this);
            watches_.clear();
            target_.clear();
        }

        void FilePathWatcherImpl::RecurciveUpdateWatches(boost::filesystem::path path, WatchEntry watch_entry) {
            InotifyReader::Watch old_watch = watch_entry.watch;
            watch_entry.watch = InotifyReader::INVALID_WATCH;
            watch_entry.linkname.clear();
            watch_entry.full_path = path;
            watch_entry.watch = InotifyReader::Get().AddWatch(path, this);
            watches_.push_back(watch_entry);
            if (watch_entry.watch == InotifyReader::INVALID_WATCH) {
                // Ignore the error code (beyond symlink handling) to attempt to add
                // watches on accessible children of unreadable directories. Note that
                // we may not catch events in this scenario.
                if (boost::filesystem::symbolic_link_exists(path))
                    AddWatchForBrokenSymlink(path, &watch_entry);
            }
            for (auto i = boost::filesystem::directory_iterator(path);
                 i != boost::filesystem::directory_iterator(); ++i) {
                if (boost::filesystem::is_directory(i->path())) {
                    auto index_of_file_begin = i->path().string().rfind('/');
                    auto ii = WatchEntry(i->path().string().substr(index_of_file_begin + 1));
                    RecurciveUpdateWatches(i->path(), ii);
                }
            }
        }

        void FilePathWatcherImpl::UpdateWatches() {
            assert(HasValidWatchVector());
            watches_.pop_back();
            // Walk the list of watches and update them as we go.
            FilePath path("/");
            for (WatchEntry &watch_entry : watches_) {
                InotifyReader::Watch old_watch = watch_entry.watch;
                watch_entry.watch = InotifyReader::INVALID_WATCH;
                watch_entry.linkname.clear();
                watch_entry.full_path = path;
                watch_entry.watch = InotifyReader::Get().AddWatch(path, this);
                if (watch_entry.watch == InotifyReader::INVALID_WATCH) {
                    // Ignore the error code (beyond symlink handling) to attempt to add
                    // watches on accessible children of unreadable directories. Note that
                    // we may not catch events in this scenario.
                    if (boost::filesystem::symbolic_link_exists(path))
                        AddWatchForBrokenSymlink(path, &watch_entry);
                }
                if (old_watch != watch_entry.watch)
                    InotifyReader::Get().RemoveWatch(old_watch, this);
                path = path / watch_entry.subdir;
            }
            auto index_of_file_begin = path.string().rfind('/');
            auto ii = WatchEntry(path.string().substr(index_of_file_begin + 1));
            RecurciveUpdateWatches(path, ii);
            for (auto i = boost::filesystem::directory_iterator(path);
                 i != boost::filesystem::directory_iterator(); ++i) {
                if (boost::filesystem::is_directory(i->path())) {
                    auto index_of_file_begin = i->path().string().rfind('/');
                    auto ii = WatchEntry(i->path().string().substr(index_of_file_begin + 1));
                    RecurciveUpdateWatches(i->path(), ii);
                }
            }
            watches_.push_back(WatchEntry(std::string()));
        }

        InotifyReader::InotifyReader() :
                inotify_fd_(inotify_init()) {
            if (inotify_fd_ < 0)
                return;

            if (!StartThread())
                return;

            valid_ = true;
        }

        bool InotifyReader::StartThread() {
            std::thread t(std::bind(&InotifyReader::ThreadMain, this));
            if (!t.joinable())
                return false;
            t.detach();
            return true;
        }

        void InotifyReader::ThreadMain() {
            while (true) {
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(inotify_fd_, &rfds);

                // Wait until some inotify events are available.
                const int select_result =
                        HANDLE_EINTR(select(inotify_fd_ + 1, &rfds, nullptr, nullptr, nullptr));
                if (select_result < 0)
                    return;

                // Adjust buffer size to current event queue size.
                int buffer_size = 0;
                const int ioctl_result = HANDLE_EINTR(ioctl(inotify_fd_, FIONREAD, &buffer_size));

                if (ioctl_result != 0)
                    return;

                std::vector<char> buffer(buffer_size);

                const ssize_t bytes_read =
                        HANDLE_EINTR(read(inotify_fd_, &buffer[0], buffer_size));

                if (bytes_read < 0)
                    return;

                ssize_t i = 0;
                while (i < bytes_read) {
                    inotify_event *event = reinterpret_cast<inotify_event *>(&buffer[i]);
                    size_t event_size = sizeof(inotify_event) + event->len;
                    assert(i + event_size <= static_cast<size_t>(bytes_read));
                    OnInotifyEvent(event);
                    i += event_size;
                }
            }
        }

        InotifyReader::Watch InotifyReader::AddWatch(
                const FilePath &path, FilePathWatcherImpl *watcher) {
            if (!valid_)
                return INVALID_WATCH;

            const Watch watch = inotify_add_watch(inotify_fd_, path.c_str(),
                                                  IN_CREATE | IN_DELETE |
                                                  IN_CLOSE_WRITE | IN_MOVE | IN_ONLYDIR);
          //  std::cout << "#################----" << watch << "---------" << path.string() << std::endl;
            watchers_[watch].insert(watcher);

            return watch;
        }

        void InotifyReader::RemoveWatch(Watch
        watch,
        FilePathWatcherImpl *watcher
        ) {
        if (!valid_ || (watch == INVALID_WATCH))
        return;

        watchers_[watch].
        erase(watcher);

        if (watchers_[watch].

        empty()

        ) {
        watchers_.
        erase(watch);

        inotify_rm_watch(inotify_fd_, watch);
    }
}

void InotifyReader::OnInotifyEvent(const inotify_event *event) {
    if (event->mask & IN_IGNORED)
        return;

    const std::string child(event->len ? event->name : "");

    std::unique_lock<std::recursive_mutex> lock(GetMutex());

    const auto watcher_set = watchers_[event->wd];
    for (FilePathWatcherImpl *watcher : watcher_set) {
        watcher->OnFilePathChanged(
                event->wd, child, event->mask & (IN_CREATE | IN_MOVED_TO),
                event->mask & (IN_DELETE | IN_MOVED_FROM),
                event->mask & (IN_ISDIR));
    }
}

void FilePathWatcherImpl::AddWatchForBrokenSymlink(const FilePath &path,
                                                   WatchEntry *watch_entry) {
    assert(InotifyReader::INVALID_WATCH >= watch_entry->watch);
    boost::system::error_code ec;
    FilePath link = boost::filesystem::read_symlink(path, ec);
    if (ec)
        return;

    if (!link.is_absolute())
        link = path.parent_path() / link;

    // Try watching symlink target directory. If the link target is "/", then we
    // shouldn't get here in normal situations and if we do, we'd watch "/" for
    // changes to a component "/" which is harmless so no special treatment of
    // this case is required.
    InotifyReader::Watch watch =
            InotifyReader::Get().AddWatch(link.parent_path(), this);
    if (watch == InotifyReader::INVALID_WATCH) {
        // Symlinks only work if the parent directory for the target
        // exist. Ideally we should make sure we've watched all the components of
        // the symlink path for changes.
        return;
    }
    watch_entry->watch = watch;
    watch_entry->linkname = link.leaf().c_str();
    watch_entry->full_path = link.parent_path();
}

bool FilePathWatcherImpl::HasValidWatchVector() const {
    if (watches_.empty())
        return false;
    for (size_t i = 0; i < watches_.size() - 1; ++i) {
        if (watches_[i].subdir.empty())
            return false;
    }
    return watches_.back().subdir.empty();
}

}  // namespace
}  // namespace gogo

namespace gogo {

    FilePathWatcher::FilePathWatcher() {
        InotifyReader::Get();
        impl_ = std::make_unique<FilePathWatcherImpl>();
    }

    FilePathWatcher::~FilePathWatcher() {
        Stop();
    }

    bool FilePathWatcher::Watch(const FilePath &path, const Callback &callback) {
        return impl_->Watch(path, callback);
    }

    void FilePathWatcher::Stop() {
        impl_->Cancel();
    }

}  // namespace gogo
