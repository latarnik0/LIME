cmake_minimum_required(VERSION 3.16)
project(lime_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(Threads REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/lime_core.cpp
)

qt_add_executable(lime_app
    ${PROJECT_SOURCES}
)

qt_add_qml_module(${PROJECT_NAME}
    URI LimeModule
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Threads::Threads
)