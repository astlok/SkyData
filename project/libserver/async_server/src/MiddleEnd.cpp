#include "MiddleEnd.h"

#include <utility>

// NOTE: Start accept in main code!
MiddleEnd::MiddleEnd(boost::asio::io_context& io_context,
                     const boost::asio::ip::tcp::endpoint& endpoint)
: AbstractServer(io_context, endpoint) { ; }

void MiddleEnd::set_client_for_backend(std::shared_ptr<ToBackendConnection> client) {
    m_backend = std::move(client);
}

void MiddleEnd::start_accept() {
    boost::shared_ptr<ServerConnection> new_connect(new ServerConnection(m_io_context,
                                                      std::shared_ptr<AbstractServer>(this)));
    m_acceptor.async_accept(new_connect->socket(),
                            boost::bind(
                                        &MiddleEnd::handle_accept,
                                        this,
                                        new_connect,
                                        boost::asio::placeholders::error));
}

void MiddleEnd::deliver_for_all(std::string msg) {
    std::for_each(m_connections.begin(), m_connections.end(),
                  boost::bind(&ServerConnection::deliver, _1, boost::ref(msg)));
}

void MiddleEnd::write_to_backend(char* msg) {
    m_backend->write(msg);
}


void MiddleEnd::on_read_message(char* msg) {
    write_to_backend(msg);
}

void MiddleEnd::send_message_if_connected(const std::string &connectionID) {
    for (auto& connection : m_connections) {
//        std::cout << "Messages amount for " <<
//                      connection->id << ": " <<
//                      QueueManager::queue_manager().get_client_messages_amount(connection->id) <<
//                      std::endl << std::endl;
        if (connection->id == connectionID) {
            std::string msg = QueueManager::queue_manager().pop_from_client_queue(connectionID);
            boost::bind(&ServerConnection::deliver, _1, msg)(connection);
            break;
        }
    }
}

