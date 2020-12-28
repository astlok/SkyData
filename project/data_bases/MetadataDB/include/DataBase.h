#ifndef PROJECT_DATA_BASES_METADATADB_INCLUDE_DATABASE_H_
#define PROJECT_DATA_BASES_METADATADB_INCLUDE_DATABASE_H_

#include "Message.h"

#include <vector>
#include <string>

class DataBase {
 public:
    DataBase() = default;

    virtual ~DataBase() = default;

    virtual std::vector<Message> update(Message &message) = 0;

    virtual bool open(std::string& config) = 0;

    virtual void close() = 0;
};

#endif  // PROJECT_DATA_BASES_METADATADB_INCLUDE_DATABASE_H_
