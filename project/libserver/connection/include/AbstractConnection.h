#ifndef PROJECT_LIBSERVER_CONNECTION_INCLUDE_ABSTRACTCONNECTION_H_
#define PROJECT_LIBSERVER_CONNECTION_INCLUDE_ABSTRACTCONNECTION_H_

#include <cstdlib>
#include <deque>
#include <string>
#include <iostream>
#include <chrono>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Message.h"

using boost::asio::ip::tcp;
typedef std::deque<std::string> chat_message_queue;
const int BUFFER_SIZE = 32768;

class AbstractConnection {
 public:
    explicit AbstractConnection(boost::asio::io_context& io_context) : m_socket(io_context), id("") { ; }

 protected:
    virtual void handle_read(const boost::system::error_code& error) = 0;
    virtual void handle_write(const boost::system::error_code& error) = 0;
    virtual void do_close() {
        boost::system::error_code err;
        m_socket.close(err);
    }

 protected:
    boost::asio::ip::tcp::socket m_socket;
    char m_read_msg[BUFFER_SIZE];
    std::deque<std::string> m_write_msgs;
    bool isConnected;

 public:
    std::string id;
    std::string last_success_message_send;
};


#endif  // PROJECT_LIBSERVER_CONNECTION_INCLUDE_ABSTRACTCONNECTION_H_
