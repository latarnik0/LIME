cmake_minimum_required(VERSION 3.16)
project(lime_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Quick3D)
find_package(Threads REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/lime_core/lime_core.cpp
    src/lime_core/lime_core.hpp
    src/lime_core/structs.hpp
    src/lime_qml_controller/lime_qml_controller.cpp
    src/lime_qml_controller/lime_qml_controller.hpp
)

qt_add_executable(lime_app
    ${PROJECT_SOURCES}
)

set_source_files_properties(qml_gui/Main.qml
    PROPERTIES QT_RESOURCE_ALIAS "Main.qml")
set_source_files_properties(qml_gui/IoChart.qml
    PROPERTIES QT_RESOURCE_ALIAS "IoChart.qml")

qt_add_qml_module(${PROJECT_NAME}
    URI LimeModule
    VERSION 1.0
    QML_FILES
        qml_gui/Main.qml
        qml_gui/IoChart.qml
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lime_core
    ${CMAKE_SOURCE_DIR}/src/lime_qml_controller
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Quick3D
    Threads::Threads
)
