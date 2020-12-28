#ifndef PROJECT_QUEUE_SERVER_INCLUDE_QUEUEMANAGER_H_
#define PROJECT_QUEUE_SERVER_INCLUDE_QUEUEMANAGER_H_

#include <map>
#include <string>

#include "Queue.h"
#include "StdQueue.h"

const char DEFAULT_DIR_STRING[] = "./";

enum engine_types {
    STD_QUEUE = 0,
    ON_DISK_QUEUE,
};

class QueueManager {
 private:
    Queue* sync_service_queue;
    std::map<std::string, Queue*> clients_queues;
    QueueManager() : sync_service_queue(nullptr),
                     sync_service_queue_type(STD_QUEUE),
                     client_queues_type(STD_QUEUE) { }

    engine_types sync_service_queue_type;
    engine_types client_queues_type;
    static Queue* createNewQueue(engine_types type);

 public:
    static QueueManager &queue_manager() {
        static QueueManager qm;
        return qm;
    }
    ~QueueManager() {
        delete sync_service_queue;
    }

    void set_sync_queue_type(engine_types type);
    void set_client_queue_type(engine_types type);
    void push_to_syncserv_queue(std::string msg);
    void create_queue_if_not_exists(const std::string& id);
    int get_client_messages_amount(const std::string& id);
    void push_to_client_queue(const std::string& msg, const std::string& id);
    std::string pop_from_syncserv_queue();
    std::string pop_from_client_queue(const std::string& id);
    bool is_syncserv_queue_empty();
    bool is_user_queue_empty(const std::string& id);
    bool is_user_queue_exists(const std::string& id);
};


#endif  // PROJECT_QUEUE_SERVER_INCLUDE_QUEUEMANAGER_H_
