set (
        OBSERVER_SRC
        observer/src/LocalListener.cpp
        observer/src/Observer.cpp
        observer/src/MessageListener.cpp
        observer/src/MessageUpdater.cpp
        observer/src/EventListen.cpp
)

set (
        INDEXER_SRC
        indexer/src/ClientSender.cpp
        indexer/src/CloudStorageWorker.cpp
        indexer/src/InternalDataBaseWorker.cpp
)

set (
        DATABASE_SRC
        ../data_bases/InternalDB/src/SQLite.cpp)

set (
        WIDGETS_SRC
        ./widgets/MainWidget.cpp)

include_directories(./widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu/qt5)
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
find_package(Qt5 COMPONENTS Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

add_executable(client main.cpp ${INDEXER_SRC} ${DB_SRC} ${OBSERVER_SRC} ${DATABASE_SRC} ${WIDGETS_SRC} ../common/Message.cpp)
target_link_libraries(client ${Boost_LIBRARIES} libserver sqlite3 liblistener Qt5::Widgets)