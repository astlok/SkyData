#include <fstream>
#include <filesystem>
#include "ClientToStorageConnection.h"

ClientToStorageConnection::ClientToStorageConnection(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoint)
: AbstractConnection(io_context),
  m_io_context(io_context), endpoint(endpoint) {
    boost::asio::async_connect(m_socket, endpoint,
                               boost::bind(
                                       &ClientToStorageConnection::handle_connect,
                                       this,
                                       boost::asio::placeholders::error));
}

void ClientToStorageConnection::write(const Message& msg) {
    boost::asio::post(m_io_context,
                      boost::bind(&ClientToStorageConnection::do_write, this, msg, false));
}

void ClientToStorageConnection::close() {
    boost::asio::post(m_io_context,
                      boost::bind(&ClientToStorageConnection::do_close, this));
    std::this_thread::sleep_for(2000ms);
    m_io_context.stop();
}

void ClientToStorageConnection::handle_connect(const boost::system::error_code& error) {
    if (!error) {
        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&](const boost::system::error_code &err, size_t bytes) {
                                    return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes;
                                },
                                boost::bind(&ClientToStorageConnection::handle_read,
                                            this,
                                            boost::asio::placeholders::error));
    } else {
        std::cerr << "ERROR: ClientToStorageConnection::handle_connect" << std::endl;
        std::this_thread::sleep_for(1000ms);
        // Trying to reconnect!
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ClientToStorageConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}

void ClientToStorageConnection::handle_read(const boost::system::error_code& error) {
    if (!error) {
        long offset = static_cast<char *>(memchr(m_read_msg, '\b', BUFFER_SIZE)) - m_read_msg;
        std::string raw_string(m_read_msg, offset);
        std::stringstream str(raw_string);
        boost::archive::text_iarchive iarch(str);
        Message msg;
        iarch >> msg;

        if (!std::filesystem::exists(msg.user.devise.sync_folder + "/" + msg.file_path)) {
            std::filesystem::create_directories(msg.user.devise.sync_folder + "/" + msg.file_path);
        }

        std::fstream file;
        if (!msg.file_path.empty()) {
            file.open(
                    msg.user.devise.sync_folder + "/" +
                    msg.file_path + "/" +
                    msg.file_name + msg.file_extension,
                    std::ios::binary | std::ios::out);
        } else {
            file.open(
                    msg.user.devise.sync_folder + "/" +
                    msg.file_name + msg.file_extension,
                    std::ios::binary | std::ios::out);
        }
        file.write(reinterpret_cast<char*>(&msg.RAW_BYTES[0]), msg.RAW_BYTES.size());
        file.close();

        boost::asio::async_read(m_socket,
                                boost::asio::buffer(m_read_msg),
                                [&] (const boost::system::error_code & err, size_t bytes)
                                { return std::find(m_read_msg, m_read_msg + bytes, '\b') < m_read_msg + bytes; },
                                boost::bind(
                                        &ClientToStorageConnection::handle_read,
                                        this,
                                        boost::asio::placeholders::error));
    } else {
        do_close();
        std::cerr << "ERROR: ClientToStorageConnection::handle_write" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        boost::asio::async_connect(m_socket, endpoint,
                                   boost::bind(
                                           &ClientToStorageConnection::handle_connect,
                                           this,
                                           boost::asio::placeholders::error));
    }
}

void ClientToStorageConnection::do_write(Message msg, bool continue_writing) {
    bool write_in_progress = !m_write_msgs.empty();
    // Serialize our Message struct to string
    if (!continue_writing) {
        msg.RAW_BYTES.clear();
        if (msg.status == PUSH_FILE) {
            std::fstream file;
            file.open(
                    msg.user.devise.sync_folder + "/" +
                    msg.file_path + "/" +
                    msg.file_name + msg.file_extension,
                    std::ios::binary | std::ios::in);
            char c = file.get();
            while (file.good()) {
                msg.RAW_BYTES.push_back(c);
                c = file.get();
            }
        }
        std::stringstream str;
        boost::archive::text_oarchive oarch(str);
        oarch << msg;
        std::cout << msg;
        // The "\b" is a SEPARATOR FOR MESSAGES!
        m_write_msgs.push_back(str.str() + '\b');
    }
    if (!write_in_progress) {
        boost::asio::async_write(m_socket,
                                 boost::asio::buffer(m_write_msgs.front().data(),
                                                      m_write_msgs.front().length()),
                                 boost::bind(&ClientToStorageConnection::handle_write, this,
                                             boost::asio::placeholders::error));
    }
}

void ClientToStorageConnection::handle_write(const boost::system::error_code& error) {
    if (!error) {
        m_write_msgs.pop_front();
        if (!m_write_msgs.empty()) {
        boost::asio::async_write(m_socket,
                boost::asio::buffer(m_write_msgs.front().data(),
                                    m_write_msgs.front().length()),
                boost::bind(&ClientToStorageConnection::handle_write, this,
                            boost::asio::placeholders::error));
        }
    } else {
        do_close();
        std::cerr << "ERROR: ClientToStorageConnection::handle_write" << std::endl;
        std::cerr << "Trying to reconnect!" << std::endl;
        boost::asio::async_connect(m_socket, endpoint,
                boost::bind(
                &ClientToStorageConnection::handle_connect,
                this,
                boost::asio::placeholders::error));
    }
}
