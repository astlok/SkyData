#include <iostream>
#include <fstream>
#include <sstream>

#include "PostgressDB.h"
#include "BackendServer.h"
#include "ServerConnection.h"
#include "Message.h"

void run_context(boost::asio::io_context& io_context) {
    io_context.run();
}

int main() {
    std::ifstream fin("db_config.txt");
    std::stringstream ss;
    ss << fin.rdbuf();
    std::string conf;
    conf = ss.str();
    fin.close();

    auto db = std::make_shared<PostgressDB>();
    db->open(conf);

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 7778);
    BackendServer server(io_context, endpoint, db);
    server.start_accept();

    boost::thread_group tg;
    for (int i = 0; i < 5; i++) {
        auto* th (new boost::thread(&run_context, boost::ref(io_context)));
        tg.add_thread(th);
    }

    tg.join_all();

    db->close();

    return 0;
}
