#include <sys/stat.h>
#include "LocalListener.h"

std::string path_str;
std::string user_name;
std::string email;
std::string sync_folder;
int quota_limit;
std::string device_name;

std::shared_ptr<Message> LocalListener::create_message(std::string path, gogo::FilePathWatcher::Event event) {
    std::shared_ptr<Message> message = std::make_shared<Message>();
    auto index_of_file_begin = path.rfind('/');
    if (index_of_file_begin == std::string::npos) {
        return nullptr;
    }

    std::string path1 = path;
    message->file_path = path1.erase(0, sync_folder.length() + 1);
    auto file_name_begin = message->file_path.rfind('/');
    if (file_name_begin == std::string::npos) {
        message->file_path = "";
    } else {
        message->file_path = message->file_path.substr(0, file_name_begin);
    }
    std::cout << message->file_path << std::endl;
    auto index_of_dote = path.rfind('.');

    if (index_of_dote == std::string::npos) {
        message->file_name = path.substr(index_of_file_begin + 1);
        message->file_extension = "";
    } else {
        message->file_name = path.substr(index_of_file_begin + 1, path.rfind('.') - index_of_file_begin - 1);
        message->file_extension = path.substr(index_of_dote);
    }
    message->file_size = get_file_size(path);
    message->times_modified = 0;

    if (event == gogo::FilePathWatcher::Event::DELETED) {
        message->status = DELETE;
    }

    if (event == gogo::FilePathWatcher::Event::CREATED) {
        message->status = CREATE;
    }

    if (event == gogo::FilePathWatcher::Event::MODIFIED) {
        message->status = MODIFIED;
    }

    message->user.user_name = user_name;
    message->user.email = email;
    message->user.devise.sync_folder = sync_folder;
    message->user.quota_limit = quota_limit;
    message->user.devise.device_name = device_name;

    if (event == gogo::FilePathWatcher::Event::NO_EVENT) {
        return nullptr;
    }

    return message;
}

int LocalListener::event_listen(const std::string& path_str_, const std::string& user_name_,
                                const std::string& email_, const std::string& sync_folder_, int quota_limit_, const std::string& device_name_) {
    path_str = path_str_;
    user_name = user_name_;
    email = email_;
    sync_folder = sync_folder_;
    quota_limit = quota_limit_;
    device_name = device_name_;

    boost::filesystem::path path = path_str;
    std::shared_ptr<NotificationReceiver> er = std::make_shared<NotificationReceiver>();
    const auto callback = std::bind(&NotificationReceiver::OnFilePathChanged,
                                    er->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    gogo::FilePathWatcher file_path_watcher;
    file_path_watcher.Watch(path, callback);

    while (true) {
        sleep(10000);
    }
}

long LocalListener::get_file_size(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


void NotificationReceiver::OnFilePathChanged(const gogo::FilePath &path, bool error,
                                             gogo::FilePathWatcher::Event event) {
    if (path.string().find("/.goutputstream") != std::string::npos) {
        return;
    }
    if (error || event == gogo::FilePathWatcher::Event::NO_EVENT) {
        return;
    }
    std::shared_ptr<Message> mes = LocalListener::create_message(path.string(), event);
    if (mes != nullptr) {
        MessageUpdater::push(mes, true);
    }
}
