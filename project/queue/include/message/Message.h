#ifndef QUEUE_MESSAGE_H
#define QUEUE_MESSAGE_H

#include <iostream>

struct devise {
    std::string device_name;
    std::string sync_folder;
} typedef devise;

struct user_t {
    std::string user_name;
    std::string email;
    int quota_limit;
} typedef user;

enum status_t {
    DELETE,
    MODIFIED,
    LOAD
};

struct Message {
    size_t version;
    status_t status;
    bool if_folder;
    size_t times_modified;
    std::string file_name;
    std::string file_extension;
    size_t file_size;
    std::string file_path;
    user_t user;
} typedef message;

#endif //QUEUE_MESSAGE_H
