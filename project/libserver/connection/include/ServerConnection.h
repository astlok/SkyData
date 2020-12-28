#ifndef PROJECT_LIBSERVER_CONNECTION_INCLUDE_SERVERCONNECTION_H_
#define PROJECT_LIBSERVER_CONNECTION_INCLUDE_SERVERCONNECTION_H_

#include <memory>
#include <string>

#include "Message.h"
#include "AbstractConnection.h"

class AbstractServer;

class ServerConnection : public AbstractConnection, public boost::enable_shared_from_this<ServerConnection> {
 public:
    ServerConnection(boost::asio::io_context& io_context, std::shared_ptr<AbstractServer> srvr);
    virtual ~ServerConnection() = default;

    boost::asio::ip::tcp::socket& socket();
    void start();
    void handle_read(const boost::system::error_code& error) override;
    void deliver(std::string msg);
    void find_file_and_send(const std::string& directory, Message msg, const std::string& storage_directory);
    void handle_write(const boost::system::error_code& error) override;
    void do_write(const std::string& msg = "");

 private:
    std::shared_ptr<AbstractServer> m_server_ptr;
};


#endif  // PROJECT_LIBSERVER_CONNECTION_INCLUDE_SERVERCONNECTION_H_
