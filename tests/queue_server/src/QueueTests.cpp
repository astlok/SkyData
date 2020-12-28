#include <limits.h>
#include "gtest/gtest.h"

#include <iostream>
#include <string>

#include "QueueManager.h"

using namespace std;

TEST(queue, correct_work) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(push_in_queue, "Denya"));
    ASSERT_EQ(QueueManager::queue_manager().pop_from_client_queue("Denya"), push_in_queue);
}

TEST(queue, pop_from_empty_queue) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(push_in_queue, "Denya"));
    ASSERT_EQ(QueueManager::queue_manager().pop_from_client_queue("Denya"), push_in_queue);
    ASSERT_ANY_THROW(QueueManager::queue_manager().pop_from_client_queue("Denya"));
}