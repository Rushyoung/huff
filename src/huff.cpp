#include "huff.hpp"

#include <iostream>
#include <fstream>
#include <string>

int huff::compress(std::string file, std::string output, bool debug) {
    std::ifstream in(file, std::ios::binary);
    if(not in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return 1;
    }

    std::ofstream out(output, std::ios::binary);
    if(not out.is_open()) {
        std::cerr << "Error opening file: " << output << std::endl;
        return 1;
    }

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