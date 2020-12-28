#include <limits.h>
#include "gtest/gtest.h"

#include <iostream>
#include <string>

#include "QueueManager.h"

using namespace std;

TEST(manager_queue, set_sync_queue_type) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().set_sync_queue_type(STD_QUEUE));
}

TEST(manager_queue, set_client_queue_type) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().set_client_queue_type(STD_QUEUE));
}

TEST(manager_queue, push_to_syncserv_queue) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_syncserv_queue(push_in_queue));
}

TEST(manager_queue, create_queue_if_not_exists) {
    std::string push_in_queue = "testing queue value";
    ASSERT_NO_THROW(QueueManager::queue_manager().create_queue_if_not_exists("Denya"));
    ASSERT_NO_THROW(QueueManager::queue_manager().create_queue_if_not_exists("Denya"));
}

TEST(manager_queue, get_client_messages_amount) {
    std::string push_in_queue = "testing queue value";
    EXPECT_EQ(QueueManager::queue_manager().get_client_messages_amount("non-exist"), 0);
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(push_in_queue, "non-exist"));
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(push_in_queue, "non-exist"));
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(push_in_queue, "non-exist"));
    EXPECT_EQ(QueueManager::queue_manager().get_client_messages_amount("non-exist"), 3);
}


TEST(manager_queue, pop_from_client_queue) {
    std::string message1 = "message1";
    std::string message2 = "message2";
    std::string message3 = "message3";
    EXPECT_EQ(QueueManager::queue_manager().get_client_messages_amount("user1"), 0);
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(message1, "user1"));
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(message2, "user1"));
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(message3, "user1"));
    EXPECT_EQ(QueueManager::queue_manager().pop_from_client_queue("user1"), message1);
    EXPECT_EQ(QueueManager::queue_manager().pop_from_client_queue("user1"), message2);
    EXPECT_EQ(QueueManager::queue_manager().pop_from_client_queue("user1"), message3);
}

TEST(manager_queue, is_user_queue_empty) {
    std::string test_message = "test";
    EXPECT_EQ(QueueManager::queue_manager().is_user_queue_empty("user2"), true);
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(test_message, "user2"));
    EXPECT_EQ(QueueManager::queue_manager().is_user_queue_empty("user2"), false);
    EXPECT_EQ(QueueManager::queue_manager().pop_from_client_queue("user2"), test_message);
    EXPECT_EQ(QueueManager::queue_manager().is_user_queue_empty("user2"), true);
}

TEST(manager_queue, is_user_queue_exists) {
    std::string test_message = "test";
    EXPECT_EQ(QueueManager::queue_manager().is_user_queue_exists("user3"), false);
    ASSERT_NO_THROW(QueueManager::queue_manager().push_to_client_queue(test_message, "user3"));
    EXPECT_EQ(QueueManager::queue_manager().is_user_queue_exists("user3"), true);
}
