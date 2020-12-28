cmake_minimum_required (VERSION 3.17)
project(liblistener)
include_directories(include)

set (
        SRC
        src/Listener.cpp
        src/utils.cpp
)

set (
        INC
        include/Listener.h
)

find_package(Boost COMPONENTS thread system serialization REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(liblistener STATIC ${SRC} ${INC})
target_link_libraries(liblistener ${Boost_LIBRARIES}  -lboost_filesystem)

target_include_directories(liblistener PUBLIC include)
