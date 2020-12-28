include_directories(client/observer/inc)
include_directories(include)
include_directories(project/common)
include_directories(project/data_bases/MetadataDB/include)

set(SYNCH_SERV_SRC
        src/main.cpp)

set(DATA_BASE_SRC
        ../data_bases/MetadataDB/src/PostgressDB.cpp)


find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

find_package(Boost COMPONENTS thread system serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(synch_service ${DATA_BASE_SRC} ${SYNCH_SERV_SRC})
target_link_libraries(synch_service ${PQXX_LIB} ${PQ_LIB} ${Boost_LIBRARIES} libserver)

