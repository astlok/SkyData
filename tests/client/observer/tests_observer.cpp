
#include "iostream"
#include "gtest/gtest.h"
#include "Observer.h"
#include "LocalListener.h"
#include "MessageListener.h"
#include "MessageUpdater.h"
#include "Downloader.h"
#include "Message.h"
#include <random>
#include "utils.h"
#include "gmock/gmock.h"
#include "EventEditor.h"
#include <cstdlib>


using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;



// Проверяет получение достоверность сообщения, полученного из очереди и записанного в message
//TEST(ListenerTest, test_on_listen_queue) {
//    Message_Listener message_listener;
//    devise_t devise = {random_str_gen(7), random_str_gen(7)};
//    user_t user = {random_str_gen(7), random_str_gen(7), devise, 5};
//    Message message = {100, MODIFIED, 0, 2, random_str_gen(7),
//                       random_str_gen(7), 100, random_str_gen(7), user};
//    Message message1 = message_listener.to_listen_message();
//
//    EXPECT_TRUE(message1 == message);
//
//}
//// Проверяет получение достоверность сообщения, полученного с локальной машины и записанного в message
//TEST(LocalListenerTest, test_on_listen_queue) {
//    Local_Listener message_listener;
//    devise_t devise = {random_str_gen(7), random_str_gen(7)};
//    user_t user = {random_str_gen(7), random_str_gen(7), devise, 7};
//    Message message = {my_random(0,200), MODIFIED, 0, 2, random_str_gen(7),
//                       random_str_gen(7), my_random(0,200), random_str_gen(7), user};
//    Message message1 = message_listener.list_local_machine();
//
//    EXPECT_TRUE(message1 == message);
//}
//
//// Проверяет правильность выполнения класса, передающего информацию обсерверу, которую нужно скачать пользователю
//TEST(Downloader, test) {
//    Cloud_Storage_Downloader<std::string> downloader;
//    std::string path_to_local_file = random_str_gen(7);
//    EXPECT_EQ(downloader.downloader_files_from_cloud(path_to_local_file), 0) ;
//
//}
//
//// Проверяет правильность выполнения главного класса, который скачивает файл на локальную машину из облака
//TEST(Observer, test_of_load) {
//    Observer<std::string> observer;
//    EXPECT_EQ(observer.load_to_machine(), 0);
//}


class MockEventEditor : public EventEditor {
public:

    MOCK_METHOD0(event, int());

};



class MockTestownloader {
public:
    MOCK_METHOD(int, downloader_files_from_cloud, (Message message), ());
    MOCK_METHOD(Message, push, (), ());
    MOCK_METHOD(Message, pop, (), ());
};

template<class Type>
class Mockownloader {
private:
    Type* MyDataBase;
    Message message;

public:
    explicit Mockownloader(Type* val) : MyDataBase(val) { }

    Message pop() {
        return MyDataBase->pop();
    }

    Message push() {
        return MyDataBase->push();
    }

    int downloader(Message message1) {
        return MyDataBase->downloader_files_from_cloud(message1);
    }

};


class MockTestMessageUpdate {
public:
    MOCK_METHOD(void, MessageUpdater, (), ());
    MOCK_METHOD(void, push, (), ());
};

template<class Type>
class MockMessageUpdate {
private:
    Type* MyClient;
    Message message;

public:
    explicit MockMessageUpdate(Type* val) : MyClient(val) { }


    void push() {
        return MyClient->push();
    }

    void update(Message message1) {
        return MyClient->MessageUpdater();
    }

};



TEST(MockDownloader, Gmock) {
    MockTestownloader mock;
    Message message = create_message();

    EXPECT_CALL(mock, downloader_files_from_cloud(message)).Times(1);
    EXPECT_CALL(mock, pop()).Times(1);
    EXPECT_CALL(mock, push()).Times(1);
    Mockownloader<MockTestownloader> mocky(&mock);
    mocky.downloader(message);
    mocky.push();
    mocky.pop();
}

TEST(MockMessageUpdate, Gmock) {
    MockTestMessageUpdate mock;
    Message message = create_message();

    EXPECT_CALL(mock, MessageUpdater).Times(1);
    EXPECT_CALL(mock, push()).Times(1);
    MockMessageUpdate<MockTestMessageUpdate> mocky(&mock);
    mocky.update(message);
    mocky.push();
}

TEST(LocalListenerTest, test_on_listen) {
    LocalListener message_listener;

    EXPECT_TRUE(message_listener.event() == 0);
}


TEST(MessageListener, test_on_listen_queue) {
    MessageListener message_listener;

    EXPECT_TRUE(message_listener.event() == 0);
}

TEST(MessageUpdater, test_on_update) {
    MessageListener message_listener;

    EXPECT_TRUE(message_listener.event() == 0);
}



int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}