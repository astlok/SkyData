set (
        INDEXER_SRC
        indexer/src/ClientSender.cpp
        indexer/src/CloudStorageWorker.cpp
        indexer/src/InternalDataBaseWorker.cpp
)

set (
        INDEXER_INC
        indexer/include/ClientSender.h
        indexer/include/CloudStorageWorker.h
        indexer/include/InternalDataBaseWorker.h
)

set (
        DB_SRC
        ../data_bases/Cloud_Storage/src/CloudStorageDB.cpp
        ../data_bases/Internal_DB/src/PostgressInternalDB.cpp
)

set (
        DB_INC
        ../data_bases/Cloud_Storage/include/CloudStorage.h
        ../data_bases/Cloud_Storage/include/CloudStorageDB.h
        ../data_bases/Internal_DB/include/InternalDataBase.h
        ../data_bases/Internal_DB/include/PostgressInternalDB.h
)

set (
        COMMANDER_SRC
        commander/src/ClientSocked.cpp
        commander/src/PushCommand.cpp
        commander/src/PopCommand.cpp
)

set (
        COMMANDER_INC
        commander/include/ClientCommand.h
        commander/include/ClientSocked.h
        commander/include/PushCommand.h
        commander/include/PopCommand.h)

set (
        OBSERVER_SRC
        observer/src/LocalListener.cpp
        observer/src/Observer.cpp
        observer/src/MessageListener.cpp
        observer/src/MessageUpdater.cpp
        observer/src/Downloader.cpp
        observer/src/EventEditor.cpp
)

set (
        DATABASE_SRC
        ../data_bases/Cloud_Storage/src/CloudStorageDB.cpp
        ../data_bases/Internal_DB/src/PostgressInternalDB.cpp
)

add_executable(project main.cpp ${INDEXER_SRC} ${DB_SRC} ${COMMANDER_SRC} ${COMMANDER_INC} ${OBSERVER_SRC} ${DATABASE_SRC})