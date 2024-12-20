cmake_minimum_required(VERSION 3.22)

# huffman compression
project(hfc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/MP)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# set the goal to windows 10 and later
add_compile_options(-DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00)

# set include directory
include_directories(include)
# set quickjs include directory
include_directories(depend/lib)


# compile src/vocab.js to src/vocab.c
# use ./depend/bin/qjsc as quickjs compiler
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.c
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/depend/bin/qjsc -c -o ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.c ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# add a custom target to ensure the command is run
add_custom_target(generate_vocab ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.c)
# add to the source files
set(vocab_GEN ${CMAKE_CURRENT_SOURCE_DIR}/src/vocab.c)
# add danamic library "depend/lib/libquickjs.a"
add_library(quickjs STATIC IMPORTED)
set_target_properties(quickjs PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/depend/lib/libquickjs.a)

# set source directory
file(GLOB SOURCES "src/*.cpp")

# add executable with sources and main.cpp file
add_executable(hfc ${SOURCES} ${vocab_GEN} main.cpp)

# link the quickjs library
target_link_libraries(hfc quickjs)

# ensure the executable depends on the generated file
add_dependencies(hfc generate_vocab)