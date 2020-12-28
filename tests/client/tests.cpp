#include <iostream>
#include "ClientsConnection.h"
#include "MessageUpdater.h"
#include "Listener.h"
#include "Observer.h"
#include "ClientSender.h"
#include "MessageListener.h"
#include "gtest/gtest.h"


std::queue<std::pair<std::shared_ptr<Message>, bool>> MessageUpdater::processed_messages = std::queue<std::pair<std::shared_ptr<Message>, bool>> ();
std::mutex MessageUpdater::mtx_stat;

//std::shared_ptr<Message> LocalListener::create_message(std::string path, gogo::FilePathWatcher::Event event)

TEST(Client, test1) {
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::CREATED;
    std::string path = "";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);

    EXPECT_EQ(mes, nullptr);
}

TEST(Client, test2) {
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::CREATED;
    std::string path = "/hello/";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);

    EXPECT_EQ(mes->file_path, "hello");
}

TEST(Client, test3) {
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::NO_EVENT;
    std::string path = "/hello/";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);

    EXPECT_EQ(mes, nullptr);
}

TEST(Client, test4) {
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::MODIFIED;
    std::string path = "/hello/";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);

    EXPECT_EQ(mes->status, MODIFIED);
}

TEST(Client, test5) {
    InternalDataBaseWorker worker;
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::MODIFIED;
    std::string path = "/hello/";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);
    int flag = worker.send_meta_data(mes);

    EXPECT_EQ(flag, 0);
}

TEST(Client, test6) {
    InternalDataBaseWorker worker;
    LocalListener loc;
    gogo::FilePathWatcher::Event ev;
    ev = gogo::FilePathWatcher::Event::MODIFIED;
    std::string path = "/hello/";
    std::shared_ptr<Message> mes = loc.create_message(path, ev);
    //int flag = 0;
    mes->status = PUSH_FILE;
    int flag = worker.send_meta_data(mes);


    EXPECT_EQ(flag, 1);
}



int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}