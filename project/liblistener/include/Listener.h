#ifndef SKYDATA_LISTENER_H
#define SKYDATA_LISTENER_H

#pragma once

#include <functional>
#include <memory>

#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

namespace gogo {
#define HANDLE_EINTR(x) ({ \
  decltype(x) eintr_wrapper_result; \
  do {                     \
  \
    eintr_wrapper_result = (x); \
  } while (eintr_wrapper_result == -1 && errno == EINTR); \
  eintr_wrapper_result; \
})

    using FilePath = boost::filesystem::path;

/*
Description:

This class |FilePathWatcher| lets you register interest in changes on a path.
The callback will get called whenever the file or directory referenced by the
path is changed, including created or deleted.

Usage:

Make sure your callback object will be alive till you watching the provided path.
One of the ways is to use std::shared_ptr and inheritance from std::enable_shared_from_this.

class NotificationReceiver : public std::enable_shared_from_this<NotificationReceiver>
{
public:
    // Callback
    void OnFilePathChanged(
        const gogo::FilePath& path,
        bool error,
        gogo::FilePathWatcher::Event event)
    {
        // event recevied
    }
};

Client code:

std::shared_ptr<NotificationReceiver> er = std::make_shared<NotificationReceiver>();
const auto callback = std::bind(&NotificationReceiver::OnFilePathChanged,
                                er->shared_from_this(), _1, _2, _3);
gogo::FilePathWatcher file_path_watcher;
file_path_watcher.Watch(path, callback);
*/

    class FilePathWatcher : private boost::noncopyable {
    public:
        // Set of events that might happen on the path that we observe
        enum class Event {
            NO_EVENT,
            CREATED,
            DELETED,
            MODIFIED
        };

        // Callback type for Watch(). |path| points to the file that was updated,
        // and |error| is true if the code detected an error. In that case, the
        // callback won't be invoked again. |event| holds information about
        // what happened on provided |path|.
        using Callback = std::function<void(const FilePath &path, bool error, Event event)>;

        // Used internally to encapsulate all logic.
        class FilePathWatcherDelegate : private boost::noncopyable {
         public:
            FilePathWatcherDelegate() = default;

            virtual ~FilePathWatcherDelegate() = default;

            // Start watching for the given |path| and notify |delegate| about changes.
            virtual bool Watch(const FilePath &path,
                               const Callback &callback) = 0;

            // Stop watching. This is called from FilePathWatcher's dtor in order to
            // allow to shut down properly while the object is still alive.
            virtual void Cancel() = 0;

         protected:
            // Must be called before the FilePathWatcherDelegate is deleted.
            void SetCancelled() {
                cancelled_ = true;
            }

            bool GetCancelled() const {
                return cancelled_;
            }

         private:
            bool cancelled_ = false;
        };

    public:
        FilePathWatcher();

        ~FilePathWatcher();

        // Invokes |callback| whenever updates to |path| are detected. This should be
        // called at most once. The callback will be invoked on the child thread.
        // Returns true on success.
        bool Watch(const FilePath &path, const Callback &callback);

        // Interrupts watching for the provided path in |Watch|
        void Stop();

    private:
        std::unique_ptr<FilePathWatcherDelegate> impl_;
    };
}  // namespace gogo

#endif  // SKYDATA_LISTENER_H
