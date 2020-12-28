#include "SQLite.h"
#include "Message.h"

int SQLite::open() {
    if (sqlite3_open("SQLite.db", &m_data_base)) {
        std::cout << "Can't open database: " << sqlite3_errmsg(m_data_base) << std::endl;
        return EXIT_FAILURE;
    }
    create_table_messages();
    return EXIT_SUCCESS;
}

void SQLite::close() {
    sqlite3_close(m_data_base);
}

bool SQLite::update(Message &message) {
    if (message.status == DELETE) {
        return del(message);
    }
    if (message.status == MODIFIED || message.status == CREATE) {
        return insert(message);
    }
    return false;
}

SQLite::~SQLite() {
    sqlite3_close(m_data_base);
}

int SQLite::create_table_messages() {
    std::string sql = "DROP TABLE IF EXISTS MESSAGES;";
    sqlite3_exec(m_data_base, sql.c_str(), callback, nullptr, nullptr);

    sql = "CREATE TABLE MESSAGES("
          "version        INT        NOT NULL,"
          "times_modified INT        NOT NULL,"
          "file_name      TEXT PRIMARY KEY,"
          "file_extension CHAR(10),"
          "file_size      INT        NOT NULL,"
          "file_path      CHAR(50));";

    int rc = sqlite3_exec(m_data_base, sql.c_str(), callback, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Create Table False" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int SQLite::callback(void *is_find, int argc, char **argv, char **azColName) {
    bool *i = reinterpret_cast<bool *>(is_find);
    *i = true;
    return EXIT_SUCCESS;
}

bool SQLite::insert(Message &message) {
    std::string comma = ",";
    std::string quotes = "'";
    std::string sql = (std::string) "INSERT INTO MESSAGES(version,times_modified,file_name,"
                      + (std::string) "file_extension, file_size, file_path) "
                      + (std::string) "VALUES (" + std::to_string(message.version) + comma
                      + std::to_string(message.times_modified) + comma
                      + quotes + message.file_name + quotes + comma
                      + quotes + message.file_extension + quotes + comma
                      + std::to_string(message.file_size) + comma
                      + quotes + message.file_path + quotes + (std::string) ");";

    if (find(message)) {
        del(message);
        message.status = MODIFIED;
    }
    int rc = sqlite3_exec(m_data_base, sql.c_str(), callback, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cout << "Insert error" << std::endl;
        return false;
    }
    return true;
}

bool SQLite::del(Message &message) {
    std::string comma = ",";
    std::string sql = (std::string) "DELETE FROM MESSAGES WHERE file_name="
                      + '"' + message.file_name + '"' + (std::string) ";";

    int rc = sqlite3_exec(m_data_base, sql.c_str(), callback, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        std::cout << "Delete error" << std::endl;
        return false;
    }
    return true;
}

SQLite::SQLite() {
    open();
}

bool SQLite::find(Message &message) {
    std::string sql = (std::string) "SELECT * FROM MESSAGES WHERE file_name="
                      + '"' + message.file_name + '"' + (std::string) ";";

    bool is_find = false;
    sqlite3_exec(m_data_base, sql.c_str(), callback, &is_find, nullptr);
    return is_find;
}

