//
// Created by denis on 03.12.2020.
//

#include "../../connection/clients_connection/clients_connection.h"
#include "../../common/Message.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

int main() {
    devise_t device{"HomePC", "/home/denis/Desktop/Folder"};
    user_t user{"Denis", "email@ml.com", device, 10};

    Message msg{10, status_t::MODIFIED, false,
                2, "main", "c",
                100, "/home/denis/Desktop/Folder", user};

    boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string("127.0.0.1"), 8001);
    std::stringstream str;
    boost::archive::text_oarchive oarch(str);
    oarch << msg;

    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "7777");

    clients_connection c(io_context, endpoints);

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

    char line[1024];
    while (std::cin.getline(line, 1024))
    {
        std::string msg_ = str.str();
        msg_ += "\n";
        c.write(msg_);
    }

    c.close();
    t.join();

    return EXIT_SUCCESS;
}