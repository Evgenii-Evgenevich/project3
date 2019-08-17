cmake_minimum_required(VERSION 3.8)

include_directories(include)

set(CMAKE_CXX_FLAGS -pthread)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if(WIN32) 
	add_subdirectory("WWindowInput")
elseif(UNIX)
	add_subdirectory("XWindowInput")
else()
	add_subdirectory("WindowInput")
endif()

add_executable(Project3 "Project3.cpp")

target_link_libraries(Project3 WindowInput)

