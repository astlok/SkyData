#ifndef PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGELISTENER_H_
#define PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGELISTENER_H_

#include <iostream>

#include "Message.h"
#include "ClientsConnection.h"
#include "ClientToStorageConnection.h"

class MessageListener {
 public:
    MessageListener(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoint,
                    const tcp::resolver::results_type& endpoint_storage);
    ClientsConnection cl_con;
    ClientToStorageConnection storage_connection;
};
#endif  // PROJECT_CLIENT_OBSERVER_INCLUDE_MESSAGELISTENER_H_
