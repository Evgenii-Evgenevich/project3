cmake_minimum_required(VERSION 3.8)

file(GLOB SOURCES "*.cpp")

add_library(WindowInput ${SOURCES})

find_package(X11 REQUIRED)

target_link_libraries(WindowInput X11)
