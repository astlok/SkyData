#include <sys/stat.h>
#include <filesystem>
#include <fstream>

#include "StorageServer.h"
#include "Message.h"

void StorageServer::start_accept() {
    boost::shared_ptr<ServerConnection> new_connect(new ServerConnection(m_io_context,
                                                      std::shared_ptr<AbstractServer>(this)));
    m_acceptor.async_accept(new_connect->socket(),
                            boost::bind(
                                    &StorageServer::handle_accept,
                                    this,
                                    new_connect,
                                    boost::asio::placeholders::error));
}

void StorageServer::set_storage_directory(std::string directory) {
    storage_directory = directory;
}

void StorageServer::deliver_for_all(std::string msg) {
    std::for_each(m_connections.begin(), m_connections.end(),
                  boost::bind(&ServerConnection::deliver, _1, boost::ref(msg)));
}

void StorageServer::on_read_message(char* msg_str) {
    std::stringstream str(msg_str);
    boost::archive::text_iarchive iarch(str);
    Message msg;
    iarch >> msg;
    if (msg.status == status_t::DOWNLOAD_FILE) {
        for (auto &connection : m_connections) {
            if (connection->id == msg.user.user_name + msg.user.devise.device_name) {
                boost::bind(&ServerConnection::find_file_and_send, _1, storage_directory, msg, storage_directory)
                           (connection);
                break;
            }
        }
    } else if (msg.status == status_t::PUSH_FILE) {
        // Check if directory exists
        if (!std::filesystem::exists(storage_directory + msg.user.user_name)) {
            std::filesystem::create_directories(storage_directory +  msg.user.user_name);
        }
        if (!std::filesystem::exists(storage_directory + msg.user.user_name + "/" + msg.file_path)) {
            std::filesystem::create_directories(storage_directory +  msg.user.user_name + "/" + msg.file_path);
        }

        std::fstream file;
        // if we will have '//' - that`s OK!
        file.open(
                storage_directory +
                msg.user.user_name + "/" +
                msg.file_path + "/" +
                msg.file_name + msg.file_extension,
                std::ios::binary | std::ios::out);
        file.write(reinterpret_cast<char*>(&msg.RAW_BYTES[0]), msg.RAW_BYTES.size());
        file.close();
    } else if (msg.status == status_t::DELETE) {
        std::filesystem::remove(
                storage_directory +
                msg.user.user_name + "/" +
                msg.file_path + "/" +
                msg.file_name + msg.file_extension);
    }
}

StorageServer::StorageServer(boost::asio::io_context &io_context, const tcp::endpoint &endpoint) : AbstractServer(
        io_context, endpoint) { ; }
