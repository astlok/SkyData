#ifndef PROJECT_CLIENT_INDEXER_INCLUDE_INTERNALDATABASEWORKER_H_
#define PROJECT_CLIENT_INDEXER_INCLUDE_INTERNALDATABASEWORKER_H_

#include <vector>
#include <memory>
#include "SQLite.h"
#include "Message.h"

class InternalDataBaseWorker {
 public:
    InternalDataBaseWorker();
    int send_meta_data(std::shared_ptr<Message> &message);

 private:
    std::shared_ptr<InternalDataBase> data_base;
};

#endif  // PROJECT_CLIENT_INDEXER_INCLUDE_INTERNALDATABASEWORKER_H_
