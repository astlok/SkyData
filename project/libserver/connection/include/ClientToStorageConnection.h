#ifndef PROJECT_LIBSERVER_CONNECTION_INCLUDE_CLIENTTOSTORAGECONNECTION_H_
#define PROJECT_LIBSERVER_CONNECTION_INCLUDE_CLIENTTOSTORAGECONNECTION_H_

#include "AbstractConnection.h"
#include "Message.h"

class ClientToStorageConnection : public AbstractConnection{
 public:
    ClientToStorageConnection(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoint);

    void write(const Message& msg);
    void close();

 private:
    void handle_connect(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error) override;
    void do_write(Message msg, bool continue_writing = false);
    void handle_write(const boost::system::error_code& error) override;

 private:
    boost::asio::io_context& m_io_context;
    tcp::resolver::results_type endpoint;
};
#endif  // PROJECT_LIBSERVER_CONNECTION_INCLUDE_CLIENTTOSTORAGECONNECTION_H_
