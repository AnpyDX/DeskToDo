cmake_minimum_required(VERSION 4.0)

project("DeskToDo"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("ext/toml11")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable("DeskToDo" WIN32 MACOSX_BUNDLE
    src/main.cxx
    src/MainWindow.h
    src/MainWindow.cxx
    src/ListItem.h
    src/ListItem.cxx
    assets/resources.rc
)

target_link_libraries("DeskToDo" PRIVATE
    Qt6::Widgets
    dwmapi
    toml11::toml11
)