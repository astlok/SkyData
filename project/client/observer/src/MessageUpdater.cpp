#include "MessageUpdater.h"
#include "ClientSender.h"
#include "ClientToStorageConnection.h"


void MessageUpdater::to_client_send(ClientsConnection &cl_con, ClientToStorageConnection &storage_conn) {
    mtx.lock();
    if (!processed_messages.empty()) {
        while (!processed_messages.empty()) {
            if (!processed_messages.front().second) {
                sender.send(processed_messages.front().first, cl_con, storage_conn, ClientSender::ONLY_SQL);
            } else {
                sender.send(processed_messages.front().first, cl_con, storage_conn, ClientSender::BOTH);
            }

            processed_messages.pop();
        }
    }
    mtx.unlock();
}

void MessageUpdater::push(const std::shared_ptr<Message> &message, bool is_from_queue) {
    mtx_stat.lock();
    processed_messages.push(std::pair<std::shared_ptr<Message>, bool>(message, is_from_queue));
    mtx_stat.unlock();
}
