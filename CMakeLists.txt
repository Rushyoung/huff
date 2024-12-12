cmake_minimum_required(VERSION 3.22)

# huffman compression
project(hfc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

if(MSVC)
    add_compile_options(/MP)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

#set the goal to windows 10 and later
add_compile_options(-DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00)

#set include directory
include_directories(include)

#set source directory
file(GLOB SOURCES "src/*.cpp")

#add executable with sources and main.c file
add_executable(hfc ${SOURCES} main.cpp)