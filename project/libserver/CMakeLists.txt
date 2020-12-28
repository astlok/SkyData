cmake_minimum_required (VERSION 3.17)

project(libserver)

include_directories(connection/include)
include_directories(async_server/include)

set(SOURCE_FILES
        async_server/src/BackendServer.cpp
        async_server/src/MiddleEnd.cpp
        async_server/src/StorageServer.cpp
        connection/src/ClientsConnection.cpp
        connection/src/ServerConnection.cpp
        connection/src/ClientToStorageConnection.cpp
        connection/src/ToBackendConnection.cpp
        ../common/Message.cpp)

set (QUEUE_SRC
        ../queue_server/src/StdQueue.cpp
        ../queue_server/src/QueueManager.cpp
        )

set(HEADER_FILES
        async_server/include/BackendServer.h
        async_server/include/MiddleEnd.h
        async_server/include/AbstractServer.h
        async_server/include/StorageServer.h
        connection/include/AbstractConnection.h
        connection/include/ClientsConnection.h
        connection/include/ServerConnection.h
        connection/include/ClientToStorageConnection.h
        connection/include/ToBackendConnection.h)


find_package(Boost COMPONENTS thread system serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(libserver STATIC ${HEADER_FILES} ${SOURCE_FILES} ${QUEUE_SRC})
target_link_libraries(libserver ${Boost_LIBRARIES})

target_include_directories(libserver PUBLIC connection async_server)
