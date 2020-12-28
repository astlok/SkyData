#include "BackendServer.h"

#include <utility>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


// NOTE: Start accept in main code!
BackendServer::BackendServer(boost::asio::io_context& io_context,
                             const boost::asio::ip::tcp::endpoint& endpoint,
                             std::shared_ptr<DataBase> data_base):
                                AbstractServer(io_context, endpoint),
                                m_data_base(data_base) { ; }

void BackendServer::start_accept() {
    boost::shared_ptr<ServerConnection> new_connect(new ServerConnection(m_io_context,
                                                      std::shared_ptr<AbstractServer>(this)));
    m_acceptor.async_accept(new_connect->socket(),
                            boost::bind(
                                        &BackendServer::handle_accept,
                                        this,
                                        new_connect,
                                        boost::asio::placeholders::error));
}

void BackendServer::deliver_for_all(std::string msg) {
    std::for_each(m_connections.begin(), m_connections.end(),
                  boost::bind(&ServerConnection::deliver, _1, boost::ref(msg)));
}


void BackendServer::on_read_message(char* msg) {
    std::shared_ptr<Message> message;
    message = deserialize(msg);
    std::vector<Message> messages = m_data_base->update(*message);

    if (!messages.empty()) {
        std::for_each(messages.begin(), messages.end(), [&](Message &mes) {
            std::cout << mes.user.devise.device_name << std::endl;
            std::string ser_mes = serialize(mes);
            ser_mes += "\b";
            deliver_for_all(ser_mes);
        });
    }
}

std::shared_ptr<Message> BackendServer::deserialize(std::string_view buf) {
    auto new_message = std::make_shared<Message>();
    std::stringstream stream;
    stream << buf;
    boost::archive::text_iarchive iarchive(stream);
    iarchive >> *new_message;

    return new_message;
}

std::string BackendServer::serialize(Message &message) {
    std::stringstream stream;
    boost::archive::text_oarchive oarchive(stream);
    oarchive << message;
    return stream.str();
}

