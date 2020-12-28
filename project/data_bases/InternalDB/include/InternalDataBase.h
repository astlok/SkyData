
#ifndef PROJECT_DATA_BASES_INTERNALDB_INCLUDE_INTERNALDATABASE_H_
#define PROJECT_DATA_BASES_INTERNALDB_INCLUDE_INTERNALDATABASE_H_

#include "Message.h"

class InternalDataBase {
 public:
    InternalDataBase() = default;
    virtual ~InternalDataBase() = default;

    virtual bool update(Message &message) = 0;

    virtual int open() = 0;
    virtual void close() = 0;
};

#endif  // PROJECT_DATA_BASES_INTERNALDB_INCLUDE_INTERNALDATABASE_H_
