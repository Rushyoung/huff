cmake_minimum_required(VERSION 3.29)
project(huff)

set(CMAKE_CXX_STANDARD 14)

add_executable(huff main.cpp
        bpe.cpp)
