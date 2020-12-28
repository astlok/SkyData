#ifndef PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGEUPDATER_H_
#define PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGEUPDATER_H_

#include <iostream>
#include <memory>
#include <mutex>
#include <utility>
#include <queue>

#include "Message.h"
#include "ClientSender.h"
#include "ClientToStorageConnection.h"


class MessageUpdater {
 private:
    std::mutex mtx;
 public:
    MessageUpdater() = default;
    static std::mutex mtx_stat;
    ClientSender sender;
 public:
    static std::queue<std::pair<std::shared_ptr<Message>, bool>> processed_messages;
    void to_client_send(ClientsConnection &cl_con, ClientToStorageConnection &storage_conn);
    static void push(const std::shared_ptr<Message>& message, bool is_from_queue = false);
};

#endif  // PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGEUPDATER_H_
