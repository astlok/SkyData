cmake_minimum_required(VERSION 3.17)
project(async_client_queue_server)


set(CMAKE_CXX_STANDARD 17)


include_directories(common)
include_directories(libserver/async_server/include)
include_directories(libserver/connection/include)

include_directories(data_bases/CloudStorage/include)
include_directories(data_bases/InternalDB/include)

include_directories(client/commander/include)
include_directories(client/observer/include)
include_directories(client/indexer/include)
include_directories(common)

include_directories(data_bases/MetadataDB/include)

include_directories(queue_server/include)

add_subdirectory(client)
add_subdirectory(synchronization_service)
add_subdirectory(storage)
add_subdirectory(queue_server)
add_subdirectory(libserver)
add_subdirectory(liblistener)
