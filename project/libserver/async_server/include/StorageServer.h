#ifndef PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_STORAGESERVER_H_
#define PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_STORAGESERVER_H_

#include <string>

#include "AbstractServer.h"

class StorageServer : public AbstractServer {
 public:
    StorageServer(boost::asio::io_context& io_context,
                  const boost::asio::ip::tcp::endpoint& endpoint);

    void start_accept() override;
    void deliver_for_all(std::string msg) override;
    void on_read_message(char* msg) override;
    void set_storage_directory(std::string directory);

    std::string storage_directory;
};

#endif  // PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_STORAGESERVER_H_
