#include "ToBackendConnection.h"

ToBackendConnection::ToBackendConnection(boost::asio::io_context& io_context,
                                       const tcp::resolver::results_type& endpoint):
                                            AbstractConnection(io_context),
                                            m_io_context(io_context), endpoint(endpoint) {
    boost::asio::async_connect(m_socket, endpoint,
                               boost::bind(
                                            &ToBackendConnection::handle_connect,
                                            this,
                                            boost::asio::placeholders::error));
}

void ToBackendConnection::set_owner_server(std::shared_ptr<MiddleEnd> serv) {
    m_server = serv;
}


void ToBackendConnection::write(const std::string& msg) {
    boost::asio::post(m_io_context,
                      boost::bind(&ToBackendConnection::do_write, this, msg));
}

void ToBackendConnection::reconnect() {
    m_socket =  boost::asio::ip::tcp::socket(m_io_context);
    boost::asio::async_connect(m_socket, endpoint,
                               boost::bind(
                                       &ToBackendConnection::handle_connect,
                                       this,
                                       boost::asio::placeholders::error));
}

void ToBackendConnection::close() {
    boost::asio::post(m_io_context,
                      boost::bind(&ToBackendConnection::do_close, this));
}

void ToBackendConnection::handle_connect(const boost::system::error_code& error) {
    if (!error) {
        isConnected = true;
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&](const boost::system::error_code &err, size_t bytes) {
                                    return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes;
                                },
                                boost::bind(&ToBackendConnection::handle_read,
                                            this,
                                            boost::asio::placeholders::error));
    } else {
        isConnected = false;
        std::cerr << "ERROR: to_backend_connection::handle_connect" << std::endl;
        std::this_thread::sleep_for(1000ms);
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ToBackendConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}

void ToBackendConnection::handle_read(const boost::system::error_code& error) {
    if (!error) {
        try {
            long offset = static_cast<char *>(memchr(m_read_msg, '\b', BUFFER_SIZE)) - m_read_msg;

            // Firstly deserialize message
            std::string str_mes(m_read_msg, offset);
            std::stringstream str(str_mes);
            boost::archive::text_iarchive iarch(str);
            Message msg;
            iarch >> msg;

            // Secondly push message to the users queue
            QueueManager::queue_manager().push_to_client_queue(std::string(m_read_msg),
                                                               std::string(msg.user.user_name
                                                                           + msg.user.devise.device_name));
//            std::cout << "IN ToBackendConnection::handle_read:: Messages amount for "
//                      << std::string(msg.user.user_name + msg.user.devise.device_name)
//                      << ": " << QueueManager::queue_manager().get_client_messages_amount(std::string(msg.user.user_name
//                                                                                                      +
//                                                                                                      msg.user.devise.device_name))
//                      << std::endl << std::endl;

            // Then call the send method to server.
            std::cout << msg.user.user_name << " " << msg.user.devise.device_name << std::endl;
            try {
                m_server->send_message_if_connected(std::string(msg.user.user_name + msg.user.devise.device_name));
            } catch (std::exception &err) {
                std::cout << err.what();
            }
            std::cout << "send " << std::endl;
            // Continue async_reading
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_read_msg),
                                    [&](const boost::system::error_code &err, size_t bytes) {
                                        return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes;
                                    },
                                    boost::bind(
                                            &ToBackendConnection::handle_read,
                                            this,
                                            boost::asio::placeholders::error));
        } catch (std::exception& err) {
            std::cerr << err.what();
        }
    } else {
        isConnected = false;
        std::cerr << "ERROR: ToBackendConnection::handle_read" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ToBackendConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}

void ToBackendConnection::do_write(const std::string& msg) {
    bool write_in_progress = !m_write_msgs.empty();
    if (!msg.empty()) { m_write_msgs.push_back(msg); }
    if (!write_in_progress && isConnected) {
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_write_msgs.front().data(),
                                                     m_write_msgs.front().length()),
                                 boost::bind(
                                                &ToBackendConnection::handle_write,
                                                this,
                                                boost::asio::placeholders::error));
    }
}

void ToBackendConnection::handle_write(const boost::system::error_code& error) {
    if (!error) {
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_write_msgs.front().data(),
                                                         m_write_msgs.front().length()),
                                     boost::bind(
                                                    &ToBackendConnection::handle_write,
                                                    this,
                                                    boost::asio::placeholders::error));
        }
    } else {
        isConnected = false;
        std::cerr << "ERROR: ToBackendConnection::handle_write" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        m_io_context.reset();
        m_socket.close();
        boost::asio::post(m_io_context,
                          boost::bind(&ToBackendConnection::reconnect, this));
    }
}
