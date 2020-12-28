#ifndef PROJECT_QUEUE_SERVER_INCLUDE_STDQUEUE_H_
#define PROJECT_QUEUE_SERVER_INCLUDE_STDQUEUE_H_

#include <queue>
#include <string>
#include <exception>
#include "Queue.h"

class StdQueue : public Queue {
 private:
    std::queue<std::string> messages;
 public:
    void push_to_queue(std::string message) override;
    std::string pop_from_queue() override;
};


#endif  // PROJECT_QUEUE_SERVER_INCLUDE_STDQUEUE_H_
