cmake_minimum_required(VERSION 3.17)
project(storage)

set(CMAKE_CXX_STANDARD 17)

set (QUEUE_SRC
        ../queue_server/src/StdQueue.cpp
        ../queue_server/src/QueueManager.cpp
        )

add_executable(storage main.cpp ${QUEUE_SRC})

find_package(Boost COMPONENTS thread system serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
include_directories(../queue_server/include)

target_link_libraries(storage ${Boost_LIBRARIES} libserver)
