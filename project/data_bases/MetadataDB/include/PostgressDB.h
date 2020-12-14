#ifndef PROJECT_POSTGRESSDB_H
#define PROJECT_POSTGRESSDB_H

#include <string_view>
#include <pqxx/pqxx>

#include "DataBase.h"
#include "Message.h"

class PostgressDB: public DataBase {
public:
     PostgressDB();

    ~PostgressDB() override;

    std::vector<Message> update(Message &message) override;

    void insert(Message &message);

    pqxx::result select(const std::string& sql_select);

    bool commit_sql_query(const std::string& sql_query);

    bool open(std::string& config);

    bool create_users_table();

    void close();

    void erase(Message &message);

private:
    std::shared_ptr<pqxx::connection> m_connect;

};

#endif //PROJECT_POSTGRESSDB_H