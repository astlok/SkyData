#ifndef PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_MIDDLEEND_H_
#define PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_MIDDLEEND_H_

#include <memory>
#include <string>

#include "AbstractServer.h"
#include "ToBackendConnection.h"
#include "ServerConnection.h"

class ToBackendConnection;

class MiddleEnd : public AbstractServer{
 public:
    explicit MiddleEnd(boost::asio::io_context& io_context,
              const boost::asio::ip::tcp::endpoint& endpoint);


    void set_client_for_backend(std::shared_ptr<ToBackendConnection>);

    virtual void start_accept() override;

    virtual void deliver_for_all(std::string msg) override;

    void send_message_if_connected(const std::string& messageID) override;

    virtual void on_read_message(char* msg) override;

    void write_to_backend(char* msg);

    void remove_connection(std::string id);

    virtual ~MiddleEnd() = default;

 private:
    std::shared_ptr<ToBackendConnection> m_backend;
};


#endif  // PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_MIDDLEEND_H_
