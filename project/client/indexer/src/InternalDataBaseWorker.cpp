#include "Message.h"
#include "InternalDataBaseWorker.h"
#include "SQLite.h"

int InternalDataBaseWorker::send_meta_data(std::shared_ptr<Message> &message) {
    if (!data_base->update(*message)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

InternalDataBaseWorker::InternalDataBaseWorker() {
    data_base = std::make_shared<SQLite>();
}
