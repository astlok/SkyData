cmake_minimum_required(VERSION 3.17)
project(server_middle)

set(CMAKE_CXX_STANDARD 17)
set (QUEUE_SRC
        ../queue_server/src/StdQueue.cpp
        ../queue_server/src/QueueManager.cpp
        )

add_executable(server_middle main.cpp ${QUEUE_SRC})

find_package(Boost COMPONENTS thread system serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(../queue_server/include)

target_link_libraries(server_middle ${Boost_LIBRARIES} libserver)

