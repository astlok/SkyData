cmake_minimum_required(VERSION 3.17)
project(queue_tests)

set(CMAKE_CXX_STANDARD 17)

set(GTEST_ROOT /usr/src/googletest/googletest)

find_package( GTest REQUIRED )
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(/usr/src/googletest/googlemock/include/gmock/)
LINK_DIRECTORIES(/usr/src/googletest/googlemock/lib)

include_directories(../../project/libserver/include
                    ../../project/common
                    ../../project/queue_server/include
                    ../../project/common)

set(TEST_SRC src/QueueTests.cpp
             src/ManagerTests.cpp)

set(QUEUE_SRC ../../project/queue_server/src/QueueManager.cpp
              ../../project/queue_server/src/StdQueue.cpp)


add_executable(queue_tests main.cpp ${TEST_SRC} ${SOCKET_SRC} ${QUEUE_SRC} ${SERVER_SRC} ${LIB_SOURCE_FILES})
target_link_libraries(queue_tests  ${GTEST_LIBRARIES} pthread -lgtest)