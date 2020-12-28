#ifndef PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_ABSTRACTSERVER_H_
#define PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_ABSTRACTSERVER_H_

#include <cstdlib>
#include <deque>
#include <set>
#include <string>
#include <memory>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "Message.h"
#include "ServerConnection.h"
#include "QueueManager.h"

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

class AbstractServer{
 public:
    AbstractServer(boost::asio::io_context& io_context, const boost::asio::ip::tcp::endpoint& endpoint):
        m_io_context(io_context),
        m_acceptor(io_context, endpoint) { ; }


    virtual void start_accept() = 0;

    void handle_accept(boost::shared_ptr<ServerConnection> session,
                       const boost::system::error_code& error) {
        if (!error) {
            std::cout << "USER CONNECTED" << std::endl;
            int32_t accept_server_socket = session->socket().native_handle();
            int32_t timeout = 2;
            int32_t cnt = 2;
            int32_t interval = 2;

            // Adding Keepalive flag
            session->socket().set_option(boost::asio::socket_base::keep_alive(true));
            setsockopt(accept_server_socket, SOL_TCP, TCP_KEEPIDLE, &timeout, sizeof(timeout));
            setsockopt(accept_server_socket, SOL_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
            setsockopt(accept_server_socket, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
            m_connections.insert(session);
            session->start();
        }

        start_accept();
    }

    virtual void deliver_for_all(std::string msg) = 0;
    virtual void on_read_message(char* msg) = 0;

    void remove_connection(std::string id, std::string message) {
        for (auto& connection : m_connections) {
            if (connection->id == id) {
                m_connections.erase(connection);
                // Pushing message, which was send unsuccessfully
                QueueManager::queue_manager().push_to_client_queue(message, connection->id);
                // The last success message sended may be not sended at all!
                if (!connection->last_success_message_send.empty()) {
                    QueueManager::queue_manager().push_to_client_queue(connection->last_success_message_send,
                                                                       connection->id);
                }
                break;
            }
        }
    }

    void remove_connection(const std::string& id) {
        for (auto& connection : m_connections) {
            if (connection->id == id) {
                m_connections.erase(connection);
                break;
            }
        }
    }


    virtual void send_message_if_connected(const std::string &connectionID) { ; }

 protected:
    boost::asio::io_context& m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    std::set<boost::shared_ptr<ServerConnection>> m_connections;
};

#endif  // PROJECT_LIBSERVER_ASYNC_SERVER_INCLUDE_ABSTRACTSERVER_H_
