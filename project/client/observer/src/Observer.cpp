#include "Observer.h"


int Observer::watch_local() {
    LocalListener local_list;
    local_list.event_listen(user.devise.sync_folder, user.user_name, user.email,
    user.devise.sync_folder, user.quota_limit, user.devise.device_name);
    return 0;
}

void Observer::update_client(const std::string &host, const std::string &port,
                             const std::string& host_storage, const std::string& port_storage) {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    tcp::resolver resolver_storage(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(host, port);
    tcp::resolver::results_type endpoints_storage = resolver_storage.resolve(host_storage, port_storage);
    MessageListener listener(io_context, endpoints, endpoints_storage);
    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));

    devise_t device_1{user.devise.device_name, ""};
    user_t user_1{user.user_name, "", device_1, 10};
    Message msg{1, status_t::LOGIN,
                1, "", "",
                1, "", user};

    listener.cl_con.write(msg);
    MessageUpdater updater;
    std::thread t1(&Observer::watch_local, this);
    while (true) {
        message_updater.to_client_send(listener.cl_con, listener.storage_connection);
        sleep(4);
    }
}

Observer::Observer(std::string user_name, std::string device, std::string sync_folder) {
    user.user_name = user_name;
    user.email = "email@";
    user.devise.sync_folder = sync_folder;
    user.quota_limit = 10;
    user.devise.device_name = device;
}
