#ifndef PROJECT_COMMON_MESSAGE_H_
#define PROJECT_COMMON_MESSAGE_H_

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <boost/serialization/access.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

struct devise_t {
    std::string device_name;
    std::string sync_folder;


 private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int vers) {
        ar & device_name;
        ar & sync_folder;
    }
} typedef devise_t;

struct user_t {
    std::string user_name;
    std::string email;
    devise_t devise;
    int quota_limit;

 private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int vers) {
        ar & user_name;
        ar & email;
        ar & devise;
        ar & quota_limit;
    }
} typedef User;

enum status_t {
    LOGIN,           // логин
    CREATE,         // новый файл
    DELETE,         // файл удален
    MODIFIED,       // файл изменен
    DOWNLOAD_FILE,  // файл загрузили
    PUSH_FILE,      // файл добавили
};

struct Message {
    size_t version;
    status_t status;
    size_t times_modified;
    std::string file_name;  // "file"
    std::string file_extension;  // ".png"
    size_t file_size;
    std::string file_path;  // "./dir/dir1/"
    user_t user;
    std::vector<char> RAW_BYTES;  // RAW_BYTES of data


    friend std::ostream& operator << (std::ostream &out, const Message &message) {
        out << "Message(" << message.version << ", " << message.status << ", "
            << message.times_modified << ", " << message.file_name << ", " << message.file_extension << ", "
            << message.file_size << ", File_path:" << message.file_path << ", "
            << message.user.email << ", " << message.user.user_name << ", " << message.user.devise.device_name <<
            ", Sync_folder:" << message.user.devise.sync_folder << ")" << std::endl << "RAW_BYTES: ";
        for (const auto& byte : message.RAW_BYTES) {
            out << byte;
        }
        out << std::endl;

        return out;
    }

 private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int vers) {
        ar & version;
        ar & status;
        ar & times_modified;
        ar & file_name;
        ar & file_extension;
        ar & file_size;
        ar & file_path;
        ar & user;
        ar & RAW_BYTES;
    }
} typedef Message;

std::string serialize(Message &message);
std::shared_ptr<Message> deserialize(const std::string &buf);
#endif  // PROJECT_COMMON_MESSAGE_H_
