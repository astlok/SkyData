#ifndef ASYNC_CLIENT_QUEUE_SERVER_BACKEND_CONNECTION_H
#define ASYNC_CLIENT_QUEUE_SERVER_BACKEND_CONNECTION_H

#include "AbstractConnection.h"
#include "MiddleEnd.h"
#include "QueueManager.h"

class MiddleEnd;

class ToBackendConnection : public AbstractConnection{
public:
    ToBackendConnection(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoint);
    virtual ~ToBackendConnection() = default;

    void write(const std::string& msg = "");
    void reconnect();
    void set_owner_server(std::shared_ptr<MiddleEnd> serv);
    void close();

private:
    void handle_connect(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error) override;
    void do_write(const std::string& msg = "");
    void handle_write(const boost::system::error_code& error) override;

private:
    boost::asio::io_context& m_io_context;
    std::shared_ptr<MiddleEnd> m_server;
    tcp::resolver::results_type endpoint;
};


#endif //ASYNC_CLIENT_QUEUE_SERVER_BACKEND_CONNECTION_H
