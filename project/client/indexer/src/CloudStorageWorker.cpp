#include "Message.h"
#include "CloudStorageWorker.h"

int CloudStorageWorker::send_to_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn) {
    status_t t = message->status;
    message->status = status_t::PUSH_FILE;
    storage_conn.write(*message);
    message->status = t;

    return EXIT_SUCCESS;
}

int CloudStorageWorker::download_from_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn) {
    status_t t = message->status;
    message->status = status_t::DOWNLOAD_FILE;
    storage_conn.write(*message);
    message->status = t;

    return EXIT_SUCCESS;
}

int CloudStorageWorker::remove_from_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn) {
    status_t t = message->status;
    message->status = status_t::DELETE;
    storage_conn.write(*message);
    message->status = t;

    return EXIT_SUCCESS;
}
