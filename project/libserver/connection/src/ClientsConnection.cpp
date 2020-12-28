#include "ClientsConnection.h"
#include "MessageUpdater.h"

ClientsConnection::ClientsConnection(
        boost::asio::io_context &io_context,
        const tcp::resolver::results_type &endpoint) :
        AbstractConnection(io_context),
        m_io_context(io_context), endpoint(endpoint) {
    boost::asio::async_connect(m_socket, endpoint,
                               boost::bind(
                                       &ClientsConnection::handle_connect,
                                       this,
                                       boost::asio::placeholders::error));
}

void ClientsConnection::write(const Message &msg) {
    boost::asio::post(m_io_context,
                      boost::bind(&ClientsConnection::do_write, this, msg, false));
}

void ClientsConnection::close() {
    boost::asio::post(m_io_context,
                      boost::bind(&ClientsConnection::do_close, this));
}

void ClientsConnection::handle_connect(const boost::system::error_code &error) {
    if (!error) {
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&](const boost::system::error_code &err, size_t bytes) {
                                    return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes;
                                },
                                boost::bind(&ClientsConnection::handle_read,
                                            this,
                                            boost::asio::placeholders::error));
    } else {
        std::this_thread::sleep_for(std::chrono_literals::operator""ms(1000));
        // Trying to reconnect!
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ClientsConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}

// Logic of file refresh
void ClientsConnection::handle_read(const boost::system::error_code &error) {
    if (!error) {
        long offset = static_cast<char *>(memchr(m_read_msg, '\b', BUFFER_SIZE)) - m_read_msg;

        // Deserialize, then push to queue
        std::shared_ptr<Message> mes = deserialize(std::string(m_read_msg, offset));
        std::cout << *mes << std::endl;
        MessageUpdater::push(mes);
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&](const boost::system::error_code &err, size_t bytes) {
                                    return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes;
                                },
                                boost::bind(
                                        &ClientsConnection::handle_read,
                                        this,
                                        boost::asio::placeholders::error));
    } else {
        std::cerr << "ERROR: ClientsConnection::handle_read" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ClientsConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error)); }
}

void ClientsConnection::do_write(const Message &msg, bool continue_writing) {
    bool write_in_progress = !m_write_msgs.empty();
    // Serialize our Message struct to string
    if (!continue_writing) {
        std::stringstream str;
        boost::archive::text_oarchive oarch(str);
        oarch << msg;
        // The "\b" is a SEPARATOR FOR MESSAGES!
        m_write_msgs.push_back(str.str() + "\b");
    }
    if (!write_in_progress) {
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_write_msgs.front().data(), m_write_msgs.front().length()),
                                 boost::bind(&ClientsConnection::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void ClientsConnection::handle_write(const boost::system::error_code &error) {
    if (!error) {
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
            boost::asio::async_write(m_socket,
                                     boost::asio::buffer(m_write_msgs.front().data(),
                                                         m_write_msgs.front().length()),
                                     boost::bind(&ClientsConnection::handle_write, this,
                                                 boost::asio::placeholders::error));
        }
    } else {
        std::cerr << "ERROR: ClientsConnection::handle_write" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ClientsConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}
