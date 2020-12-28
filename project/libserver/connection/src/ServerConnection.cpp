#include <fstream>
#include "ServerConnection.h"
#include "AbstractServer.h"
#include "Message.h"

ServerConnection::ServerConnection(boost::asio::io_context &io_context,
                                   std::shared_ptr<AbstractServer> srvr) :
        AbstractConnection(io_context),
        m_server_ptr(srvr) { ; }

boost::asio::ip::tcp::socket &ServerConnection::socket() {
    return m_socket;
}

void ServerConnection::start() {
    boost::asio::async_read(m_socket,
                            boost::asio::buffer(m_read_msg),
                            [&](const boost::system::error_code &err, size_t bytes) {
                                return (std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes ||
                                        std::find(m_read_msg, m_read_msg + bytes, EOF) < m_read_msg + bytes);
                            },
                            boost::bind(
                                    &ServerConnection::handle_read,
                                    shared_from_this(),
                                    boost::asio::placeholders::error));
}

void ServerConnection::handle_read(const boost::system::error_code &error) {
    if (!error) {
        long offset = static_cast<char *>(memchr(m_read_msg, '\b', BUFFER_SIZE)) - m_read_msg;


        std::string str_mes(m_read_msg, offset);
        std::stringstream str(str_mes);
        boost::archive::text_iarchive iarch(str);
        Message msg;
        iarch >> msg;


        if (this->id.empty()) {
            m_server_ptr->remove_connection(msg.user.user_name + msg.user.devise.device_name);
            this->id = msg.user.user_name + msg.user.devise.device_name;
            QueueManager::queue_manager().create_queue_if_not_exists(this->id);
            // Do we have messages for user in queue?
            while (QueueManager::queue_manager().is_user_queue_exists(this->id) &&
                   !QueueManager::queue_manager().is_user_queue_empty(this->id)) {
                m_server_ptr->send_message_if_connected(this->id);
            }
        }

        m_server_ptr->on_read_message(m_read_msg);

        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&](const boost::system::error_code &err, size_t bytes) {
                                    return (std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes);
                                },
                                boost::bind(
                                        &ServerConnection::handle_read,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
    } else {
        std::cerr << "handle_read ERROR: " << error.message();
        m_server_ptr->remove_connection(this->id, m_write_msgs.front());
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_write_msgs.front().data(),
                                                         m_write_msgs.front().length()),
                                     boost::bind(
                                             &ServerConnection::handle_write,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
        }
    }
}


void ServerConnection::deliver(std::string msg) {
    bool write_in_progress = !m_write_msgs.empty();
    m_write_msgs.emplace_back(msg);
    if (!write_in_progress) {
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_write_msgs.front().data(),
                                                     m_write_msgs.front().length()),
                                 boost::bind(
                                         &ServerConnection::handle_write,
                                         shared_from_this(),
                                         boost::asio::placeholders::error));
    }
}

void ServerConnection::handle_write(const boost::system::error_code &error) {
    if (!error) {
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_write_msgs.front().data(),
                                                         m_write_msgs.front().length()),
                                     boost::bind(
                                             &ServerConnection::handle_write,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
        }
    } else {
        m_server_ptr->remove_connection(this->id, m_write_msgs.front());
        std::cerr << "ServerConnection::handle_write ERROR: " << error.message();
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_write_msgs.front().data(),
                                                         m_write_msgs.front().length()),
                                     boost::bind(
                                             &ServerConnection::handle_write,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
        }
    }
}

void ServerConnection::find_file_and_send(const std::string& directory, Message msg, const std::string& storage_directory) {
    std::fstream file;
    file.open(
            storage_directory +
            msg.user.user_name + "/" +
            msg.file_path + "/" +
            msg.file_name + msg.file_extension,
            std::ios::binary | std::ios::in);
    Message msg_to_send;

    msg_to_send.version = msg.version;
    msg_to_send.status = msg.status;
    msg_to_send.times_modified = msg.times_modified;
    msg_to_send.file_name = msg.file_name;  // "file"
    msg_to_send.file_extension = msg.file_extension;  // ".png"
    msg_to_send.file_size = msg.file_size;
    msg_to_send.file_path = msg.file_path;  // "./dir/dir1/"
    msg_to_send.user = msg.user;

    char c = file.get();
    while (file.good()) {
        msg_to_send.RAW_BYTES.push_back(c);
        c = file.get();
    }
    file.close();

    std::stringstream str;
    boost::archive::text_oarchive oarch(str);
    oarch << msg_to_send;
    str << "\b";
    do_write(str.str());
}

void ServerConnection::do_write(const std::string &msg) {
    bool write_in_progress = !m_write_msgs.empty();
    if (!msg.empty()) { m_write_msgs.push_back(msg); }
    if (!write_in_progress) {
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_write_msgs.front().data(),
                                                     m_write_msgs.front().length()),
                                 boost::bind(
                                         &ServerConnection::handle_write,
                                         this,
                                         boost::asio::placeholders::error));
    }
}
