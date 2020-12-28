#ifndef PROJECT_DATA_BASES_METADATADB_INCLUDE_POSTGRESSDB_H_
#define PROJECT_DATA_BASES_METADATADB_INCLUDE_POSTGRESSDB_H_

#include <pqxx/pqxx>
#include <string_view>
#include <vector>
#include <string>
#include <memory>

#include "DataBase.h"
#include "Message.h"

class PostgressDB: public DataBase {
 public:
    PostgressDB();

    ~PostgressDB() override;

    virtual std::vector<Message> update(Message &message) override;

    virtual bool open(std::string& config) override;

    virtual void close() override;

 private:
    void insert_devise(Message &message);

    void insert_file(Message &message);

    pqxx::result select(const std::string& sql_select);

    bool modified(Message &message);

    bool commit_sql_query(const std::string& sql_query);

    bool create_users_devises_table();

    bool create_users_files_table();

    void erase(Message &message);

 private:
    std::shared_ptr<pqxx::connection> m_connect;
};

#endif  // PROJECT_DATA_BASES_METADATADB_INCLUDE_POSTGRESSDB_H_
