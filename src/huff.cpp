#include "huff.hpp"
#include "jsrt.hpp"

#include <iostream>
#include <fstream>
#include <string>

int huff::compress(std::string file, std::string output, bool debug) {
    std::ifstream in(file, std::ios::binary);
    if(not in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return 1;
    }
    in.seekg(0, std::ios::end);
    int size = in.tellg();
    in.seekg(0, std::ios::beg);
    char* buffer = new char[size + 1];
    in.read(buffer, size);
    buffer[size] = '\0';

    // compress file
    return 0;
}


int huff::decompress(std::string file, bool debug) {
    std::ifstream in(file, std::ios::binary);
    if(not in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return 1;
    }

    // decompress file
    return 0;
}