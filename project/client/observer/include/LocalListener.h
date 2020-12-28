#ifndef PROJECT_CLIENT_OBSERVER_INCLUDE_LOCALLISTENER_H_
#define PROJECT_CLIENT_OBSERVER_INCLUDE_LOCALLISTENER_H_
#include <iostream>
#include <memory>
#include <string>

#include "Message.h"
#include "EventListen.h"
#include "Listener.h"
#include "MessageUpdater.h"

class NotificationReceiver : public std::enable_shared_from_this<NotificationReceiver> {
 public:
    void OnFilePathChanged(
            const gogo::FilePath& path,
            bool error,
            gogo::FilePathWatcher::Event event);
};

class LocalListener : public EventListen{
    static long get_file_size(std::string filename);
    friend NotificationReceiver;


 public:
    static std::shared_ptr<Message> create_message(std::string path, gogo::FilePathWatcher::Event event);
    int event_listen(const std::string& path_str, const std::string& user_name,
                     const std::string&, const std::string&, int, const std::string&) override;
};


#endif  // PROJECT_CLIENT_OBSERVER_INCLUDE_LOCALLISTENER_H_
