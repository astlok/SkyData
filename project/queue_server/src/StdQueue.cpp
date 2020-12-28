#include "StdQueue.h"

void StdQueue::push_to_queue(std::string message) {
    mtx.lock();
    messages.push(message);
    size++;
    mtx.unlock();
}

std::string StdQueue::pop_from_queue() {
    mtx.lock();
    if (messages.empty()) { throw std::exception();}
    std::string msg = messages.front();
    messages.pop();
    size--;
    mtx.unlock();
    return msg;
}
