#ifndef PROJECT_CLIENT_INDEXER_INCLUDE_CLOUDSTORAGEWORKER_H_
#define PROJECT_CLIENT_INDEXER_INCLUDE_CLOUDSTORAGEWORKER_H_

#include <vector>
#include <memory>

#include "ClientToStorageConnection.h"
#include "Message.h"

class CloudStorageWorker {
 public:
    CloudStorageWorker() = default;
    int send_to_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn);
    int download_from_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn);
    int remove_from_cloud(std::shared_ptr<Message> &message, ClientToStorageConnection &storage_conn);


 private:
    std::vector<Message> *messages;
};

#endif  // PROJECT_CLIENT_INDEXER_INCLUDE_CLOUDSTORAGEWORKER_H_
