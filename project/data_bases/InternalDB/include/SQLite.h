#ifndef PROJECT_DATA_BASES_INTERNALDB_INCLUDE_SQLITE_H_
#define PROJECT_DATA_BASES_INTERNALDB_INCLUDE_SQLITE_H_

#include <sqlite3.h>
#include <memory>
#include "InternalDataBase.h"
#include "Message.h"

class SQLite: public InternalDataBase {
 public:
    SQLite();
    ~SQLite() override;

    bool update(Message &message) override;

    int open() override;
    void close() override;
    static int callback(void *NotUsed, int argc, char **argv, char **azColName);
 private:
    sqlite3 *m_data_base{};

    int create_table_messages();
    bool insert(Message &message);
    bool del(Message &message);
    bool find(Message &message);
};

#endif  // PROJECT_DATA_BASES_INTERNALDB_INCLUDE_SQLITE_H_
