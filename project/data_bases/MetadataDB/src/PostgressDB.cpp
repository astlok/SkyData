#include <pqxx/pqxx>
#include <algorithm>

#include "PostgressDB.h"

PostgressDB::PostgressDB()= default;;

PostgressDB::~PostgressDB() {
    m_connect->disconnect();
}

bool PostgressDB::open(std::string& config) {
    m_connect = std::make_shared<pqxx::connection>(config);
    create_users_files_table();
    create_users_devises_table();
    return m_connect->is_open();
}

void PostgressDB::close() {
    m_connect->disconnect();
}

void PostgressDB::insert_file(Message &message) {
    std::string sql;
    std::string quote = "'";

    sql = "INSERT INTO USERS_FILES (NAME,EMAIL,VERSION,TIMES_MODIFIED,FILE_NAME,FILE_EXTENSION,"
          "FILE_SIZE,FILE_PATH,SYNC_FOLDER,QUOTA_LIMIT) "
          "VALUES ("
          + quote + message.user.user_name + quote + ", "
          + quote + message.user.email + quote + ", "
          + std::to_string(message.version) + ", "
          + std::to_string(message.times_modified) + ", "
          + quote + message.file_name + quote + ", "
          + quote + message.file_extension + quote + ", "
          + std::to_string(message.file_size) + ", "
          + quote + message.file_path + quote + ", "
          + quote + message.user.devise.sync_folder + quote + ", "
          + std::to_string(message.user.quota_limit) + ");";
    commit_sql_query(sql);
}

void PostgressDB::insert_devise(Message &message) {
    std::string sql;
    std::string quote = "'";

    sql = "INSERT INTO USERS_DEVISES (NAME,DEVICE_NAME,SYNC_FOLDER) "
          "VALUES ("
          + quote + message.user.user_name + quote + ", "
          + quote + message.user.devise.device_name + quote + ", "
          + quote + message.user.devise.sync_folder +quote + ");";

    commit_sql_query(sql);
}

void PostgressDB::erase(Message &message) {
    std::string sql;
    std::string quote = "'";

    sql = "DELETE FROM users_files "
          "WHERE name = " + quote + message.user.user_name + quote +
          "AND file_name = " + quote + message.file_name + quote +
          "AND file_extension = " + quote + message.file_extension + quote +
          "AND file_path = " + quote + message.file_path + quote;

    commit_sql_query(sql);
}

pqxx::result PostgressDB::select(const std::string& sql_select) {
    pqxx::nontransaction nontransaction(*m_connect);

    pqxx::result pq_result(nontransaction.exec(sql_select));

    return pq_result;
}

bool PostgressDB::commit_sql_query(const std::string& sql_query) {
    try {
        pqxx::work work(*m_connect);
        work.exec(sql_query);
        work.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

std::vector<Message> PostgressDB::update(Message &message) {
    std::string quote = "'";
    std::vector<Message> messages;
    pqxx::result result;
    std::vector<std::string> vec_str;
    bool not_create = false;
    switch (message.status) {
        case LOGIN:
            result = select("SELECT * from USERS_DEVISES "
                            "WHERE device_name = " + quote + message.user.devise.device_name + quote);
            if (result.empty()) {
                result = select("SELECT * FROM users_files "
                                "WHERE name = " + quote + message.user.user_name + quote);
                for (const auto &row : result) {
                    for (const auto &field : row) {
                        vec_str.push_back(field.as<std::string>());
                    }
                }
                for (int i = 0; i < vec_str.size(); i = i + 10) {
                    Message new_message;
                    new_message.user.user_name = vec_str[i];
                    new_message.user.email = vec_str[i + 1];
                    new_message.version = std::stoi(vec_str[i + 2]);
                    new_message.times_modified = std::stoi(vec_str[i + 3]);
                    new_message.file_name = vec_str[i + 4];
                    new_message.file_extension = vec_str[i + 5];
                    new_message.file_size = std::stoi(vec_str[i + 6]);
                    new_message.file_path = vec_str[i + 7];
                    new_message.user.devise.device_name = message.user.devise.device_name;
                    new_message.user.devise.sync_folder = message.user.devise.sync_folder;
                    new_message.user.quota_limit = std::stoi(vec_str[i + 9]);
                    new_message.status = CREATE;
                    messages.push_back(new_message);
                }
                insert_devise(message);
            }
            return messages;
        case CREATE:
            result = select("SELECT * FROM users_files "
                            "WHERE file_name = " + quote + message.file_name + quote + " " +
                            "AND file_extension = " + quote + message.file_extension + quote + " "
                            "AND name = " + quote + message.user.user_name + quote);

            if (!result.empty()) {
                not_create = true;
            }

            result = select("SELECT * FROM users_devises "
                            "WHERE name = " + quote + message.user.user_name + quote + " "
                            "AND device_name != " +  quote + message.user.devise.device_name + quote);
            for (const auto &row : result) {
                for (const auto &field : row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (int i = 0; i < vec_str.size(); i = i + 3) {
                Message new_message;
                new_message = message;
                new_message.user.devise.device_name = vec_str[i + 1];
                new_message.user.devise.sync_folder = vec_str[i + 2];
                messages.push_back(new_message);
            }
            if (not_create) { return messages; }

            insert_file(message);
            return messages;

        case DELETE: case MODIFIED:
            if (message.status == MODIFIED) {
                modified(message);
            }
            result = select("SELECT * FROM users_devises "
                            "WHERE name = " + quote + message.user.user_name + quote + " " +
                            "AND device_name != "  +  quote + message.user.devise.device_name + quote);
            for (const auto &row : result) {
                for (const auto &field : row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (int i = 0; i < vec_str.size(); i = i + 3) {
                Message new_message;
                new_message = message;
                new_message.user.devise.device_name = vec_str[i + 1];
                new_message.user.devise.sync_folder = vec_str[i + 2];
                messages.push_back(new_message);
            }
            if (message.status == DELETE) {
                erase(message);
            }
            return messages;
    }
    return messages;
}

bool PostgressDB::create_users_devises_table() {
    std::string  sql;

    try {
        //  FIXME: For demonstration, delete for release
        sql = "DROP TABLE IF EXISTS USERS_DEVISES;";
        commit_sql_query(sql);

        sql = "CREATE TABLE USERS_DEVISES(" \
        "NAME           TEXT    NOT NULL," \
        "DEVICE_NAME    TEXT    NOT NULL," \
        "SYNC_FOLDER    TEXT    NOT NULL);";

        commit_sql_query(sql);
        std::cout << "Table users_devises created successfully" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool PostgressDB::create_users_files_table() {
    std::string  sql;

    try {
        //  FIXME: For demonstration, delete for release
        sql = "DROP TABLE IF EXISTS USERS_FILES;";
        commit_sql_query(sql);

        sql = "CREATE TABLE USERS_FILES(" \
        "NAME           TEXT    NOT NULL," \
        "EMAIL          TEXT    NOT NULL," \
        "VERSION        INT," \
        "TIMES_MODIFIED INT," \
        "FILE_NAME      TEXT," \
        "FILE_EXTENSION TEXT," \
        "FILE_SIZE      INT," \
        "FILE_PATH      TEXT," \
        "SYNC_FOLDER    TEXT," \
        "QUOTA_LIMIT    INT);";

        commit_sql_query(sql);
        std::cout << "Table users_files created successfully" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

bool PostgressDB::modified(Message &message) {
    std::string quote = "'";
    std::string sql = "UPDATE users_files set file_size = " + std::to_string(message.file_size) + " "
                      "WHERE file_name = " + quote + message.file_name + quote + " "
                      "AND file_extension = " + quote + message.file_extension + quote + " "
                                                                                                                                                    "AND name = " + quote + message.user.user_name + quote;
    commit_sql_query(sql);
    return false;
}
