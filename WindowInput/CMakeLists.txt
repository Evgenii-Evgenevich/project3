cmake_minimum_required(VERSION 3.8)

file(GLOB SOURCES "*.hpp")

add_library(WindowInput ${SOURCES})
