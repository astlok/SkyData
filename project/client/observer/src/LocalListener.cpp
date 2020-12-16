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
        return nullptr; // Todo добавить обработку исключений
    }
    // ToDo добавить логирование
    message->file_path = path.substr(0, index_of_file_begin);
    auto index_of_dote = path.rfind('.');

    if (index_of_dote == std::string::npos) {
        message->file_name = path.substr(index_of_file_begin + 1);
        message->file_extension = "";
    } else {
        message->file_name = path.substr(index_of_file_begin + 1, path.rfind('.') - index_of_file_begin - 1);
        message->file_extension = path.substr(index_of_dote);
    }
    message->file_size = get_file_size(path);
    message->times_modified = 0; // ToDo че нить придумать с этим полем

    if (event == gogo::FilePathWatcher::Event::DELETED) {
        message->status = DELETE;
    }

    if (event == gogo::FilePathWatcher::Event::CREATED) {
        message->status = CREATE;
    }

    if (event == gogo::FilePathWatcher::Event::MODIFIED) {
        message->status = MODIFIED;
    }

    message->file_path = path_str;
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
   // std::cout << "-------------------------------------------------------------------------------------------\n";
    std::shared_ptr<NotificationReceiver> er = std::make_shared<NotificationReceiver>();
    //std::cout << "-------------------------------------------------------------------------------------------\n";
    const auto callback = std::bind(&NotificationReceiver::OnFilePathChanged,
                                    er->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    //std::cout << "-------------------------------------------------------------------------------------------\n";
    gogo::FilePathWatcher file_path_watcher;
    // while (true) {
   // std::cout << "-------------------------------------------------------------------------------------------\n";
    file_path_watcher.Watch(path, callback);

    while (true) {
        sleep(10000);
    }


    return EXIT_SUCCESS;
}

long LocalListener::get_file_size(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


void NotificationReceiver::OnFilePathChanged(const gogo::FilePath &path, bool error,
                                             gogo::FilePathWatcher::Event event) {
    path.string();
    std::shared_ptr<Message> mes = LocalListener::create_message(path.string(), event);
    if (mes != nullptr) {
        MessageUpdater::push(mes, true);
    }
    std::cout << path << " PATH\n";

}
