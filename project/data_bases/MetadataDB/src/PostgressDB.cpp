#include "PostgressDB.h"
#include <pqxx/pqxx>
#include <algorithm>

// FIXME: remove this
void print_mes_db(Message &message) {
    std::cout << "user name: " << message.user.user_name << std::endl;
    std::cout << "user email: " << message.user.email << std::endl;
    std::cout << "version: " << message.version << std::endl;
    std::cout << "times modified: " << message.times_modified << std::endl;
    std::cout << "file name: " << message.file_name << std::endl;
    std::cout << "file extention: " << message.file_extension<< std::endl;
    std::cout << "file size: " << message.file_size << std::endl;
    std::cout << "file path: " << message.file_path << std::endl;
    std::cout << "devise name: " << message.user.devise.device_name << std::endl;
    std::cout << "sync folder: " << message.user.devise.sync_folder << std::endl;
    std::cout << "quota limit: " << message.user.quota_limit << std::endl;
}

PostgressDB::PostgressDB()= default;;

PostgressDB::~PostgressDB() {
    m_connect->close();
}

bool PostgressDB::open(std::string& config) {
    m_connect = std::make_shared<pqxx::connection>(config);
    return m_connect->is_open();
}

void PostgressDB::close() {
    m_connect->close();
}

void PostgressDB::insert(Message &message) {

    std::string sql;
    std::string quote = "'";

    sql = "INSERT INTO USERS_INFO (NAME,EMAIL,VERSION,TIMES_MODIFIED,FILE_NAME,FILE_EXTENSION,"
          "FILE_SIZE,FILE_PATH,DEVICE_NAME,SYNC_FOLDER,QUOTA_LIMIT) "
          "VALUES ("
          + quote + message.user.user_name + quote + ", "
          + quote + message.user.email + quote + ", "
          + std::to_string(message.version) + ", "
          + std::to_string(message.times_modified) + ", "
          + quote + message.file_name + quote + ", "
          + quote + message.file_extension + quote + ", "
          + std::to_string(message.file_size) + ", "
          + quote + message.file_path + quote + ", "
          + quote + message.user.devise.device_name + quote + ", "
          + quote + message.user.devise.sync_folder + quote + ", "
          + std::to_string(message.user.quota_limit) + ");";

    /* Create a transactional object. */
    pqxx::work W(*m_connect);

    /* Execute SQL query */
    W.exec(sql);
    W.commit();

}

void PostgressDB::erase(Message &message) {
    std::string sql;
    std::string quote = "'";

    sql = "DELETE FROM users_info "
          "WHERE name = " + quote + message.user.user_name + quote +
          "AND file_name = " + quote + message.file_name + quote +
          "AND file_extension = " + quote + message.file_extension + quote +
          "AND file_path = " + quote + message.file_path + quote;

    commit_sql_query(sql);
}

bool PostgressDB::create_users_table() {
    std::string  sql;

    try {
//        if (m_connect->is_open()) {
//            std::cout << "Opened database successfully: " << m_connect->dbname() << std::endl;
//        } else {
//            std::cout << "Can't open database\n";
//            return false;
//        }

        /* Create SQL statement */
        sql = "CREATE TABLE USERS_INFO(" \
        "NAME           TEXT    NOT NULL," \
        "EMAIL          TEXT    NOT NULL," \
        "VERSION        INT," \
        "TIMES_MODIFIED INT," \
        "FILE_NAME      TEXT," \
        "FILE_EXTENSION TEXT," \
        "FILE_SIZE      INT," \
        "FILE_PATH      TEXT," \
        "DEVICE_NAME    TEXT," \
        "SYNC_FOLDER    TEXT," \
        "QUOTA_LIMIT    INT);";

        commit_sql_query(sql);
        std::cout << "Table created successfully" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }

    return true;
}

pqxx::result PostgressDB::select(std::string_view sql_select) {
    pqxx::nontransaction nontransaction(*m_connect);

    pqxx::result pq_result (nontransaction.exec(sql_select));

    return pq_result;
}

bool PostgressDB::commit_sql_query(std::string_view sql_query) {
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
    std::cout << "mes status: " << message.status << std::endl;
    pqxx::result result;
    std::vector<std::string> vec_str;
    switch(message.status) {
        case NEW_USER:
            insert(message);
            return messages;
        case CREATE:
            result = select("SELECT device_name FROM users_info "
                                            "WHERE name = " + quote + message.user.user_name + quote + " "
                                            "AND device_name != " +  quote + message.user.devise.device_name + quote);
            for (const auto &row: result) {
                for (const auto &field: row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (auto & i : vec_str) {
                Message new_message;
                new_message = message;
                new_message.user.devise.device_name = i;
                messages.push_back(new_message);
            }
            print_mes_db(message);
            insert(message);
            return messages;
        case DELETE:
            result = select("SELECT device_name FROM users_info "
                            "WHERE name = " + quote + message.user.user_name + quote + " " +
                            "AND device_name != "  +  quote + message.user.devise.device_name + quote);
            for (const auto &row: result) {
                for (const auto &field: row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (auto & i : vec_str) {
                Message new_message;
                new_message = message;
                new_message.user.devise.device_name = i;
                messages.push_back(new_message);
            }
            erase(message);
            return messages;
        case MODIFIED:
            result = select("SELECT device_name FROM users_info "
                            "WHERE name = " + quote + message.user.user_name + quote + " " +
                            "AND device_name != " +  quote + message.user.devise.device_name + quote);
            for (const auto &row: result) {
                for (const auto &field: row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (auto & i : vec_str) {
                Message new_message;
                new_message = message;
                new_message.user.devise.device_name = i;
                messages.push_back(new_message);
            }
            return messages;
        case NEW_DEVISE:
            result = select("SELECT * FROM users_info "
                            "WHERE name = " + quote + message.user.user_name + quote);
            for (const auto &row: result) {
                for (const auto &field: row) {
                    vec_str.push_back(field.as<std::string>());
                }
            }
            for (int i = 0; i < vec_str.size(); i = i + 11) {
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
                // FIXME: remove this
//                new_message.user.devise.device_name = "iPhone";

                new_message.user.devise.sync_folder = vec_str[i + 9];
                new_message.user.quota_limit = std::stoi(vec_str[i + 10]);
                new_message.status = NEW_DEVISE;
                messages.push_back(new_message);
                print_mes_db(new_message);
            }
            return messages;
    }
    return messages;
}


