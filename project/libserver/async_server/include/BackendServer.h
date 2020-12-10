#ifndef ASYNC_CLIENT_QUEUE_SERVER_BACKEND_SERVER_H
#define ASYNC_CLIENT_QUEUE_SERVER_BACKEND_SERVER_H

#include "AbstractServer.h"
#include "DataBase.h"

class BackendServer : public AbstractServer {
public:

    BackendServer(boost::asio::io_context& io_context,
                  const boost::asio::ip::tcp::endpoint& endpoint,
                  std::shared_ptr<DataBase> data_base);

    virtual void start_accept() override;

    virtual void on_readed_message(char* msg) override;

    virtual ~BackendServer() = default;

    std::string serialize(Message &message);

    std::shared_ptr<Message> deserialize(std::string_view buf);

private:
    virtual void deliver_for_all(char* msg) override;

    std::shared_ptr<DataBase> m_data_base;
};


#endif //ASYNC_CLIENT_QUEUE_SERVER_BACKEND_SERVER_H
