#include "Message.h"

#include <sstream>

std::string serialize(Message &message) {
    std::stringstream stream;
    boost::archive::text_oarchive oarchive(stream);
    oarchive << message;
    return stream.str();
}

std::shared_ptr<Message> deserialize(const std::string &buf) {
    auto new_message = std::make_shared<Message>();
    std::stringstream stream;
    stream << buf;
    boost::archive::text_iarchive iarchive(stream);
    iarchive >> *new_message;

    return new_message;
}


