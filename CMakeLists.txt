cmake_minimum_required(VERSION 3.10)
project(lime)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(lime 
    src/main.cpp 
    src/lime_core.cpp 
    src/ui.cpp
)

target_link_libraries(lime ${CURSES_LIBRARIES})
