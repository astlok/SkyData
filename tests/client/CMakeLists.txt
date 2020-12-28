include_directories(../../project/client/observer/include)
include_directories("../../project/common")
include_directories(./project/client/observer)
include_directories(../../project/client/indexer/include)
include_directories(../../project/data_bases/CloudStorage/include)
include_directories(../../project/data_bases/InternalDB/include)
include_directories(../../project/common)
include_directories(../../project/libserver/async_server/include)
include_directories(../../project/libserver/connection/include)



set (
        OBSERVER_SRC
        ../../project/client/observer/src/LocalListener.cpp
        ../../project/client/observer/src/Observer.cpp
        ../../project/client/observer/src/MessageListener.cpp
        ../../project/client/observer/src/MessageUpdater.cpp
)

set (
        SRC
        ../../project/client/indexer/src/ClientSender.cpp
        ../../project/client/indexer/src/CloudStorageWorker.cpp
        ../../project/client/indexer/src/InternalDataBaseWorker.cpp
)

set (
        DB_SRC
        ../../project/data_bases/CloudStorage/src/CloudStorageDB.cpp
        ../../project/data_bases/InternalDB/src/SQLite.cpp
)

set (
        MDA
        ../../project/common/Message.cpp
)


add_executable(client_test tests.cpp  ${LIB_SOURCE_FILES} ${OBSERVER_SRC} ${SRC} ${DB_SRC} ${MDA})
target_link_libraries(client_test ${GTEST_LIBRARIES} pthread -lgmock libserver sqlite3 liblistener)