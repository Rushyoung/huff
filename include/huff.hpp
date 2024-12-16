#pragma once

#include <string>
#include <filesystem>
#include <fstream>

namespace huff{

int compress(std::string file, std::string output, bool debug=false) {
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


int decompress(std::string file, bool debug=false) {
    std::ifstream in(file, std::ios::binary);
    if(not in.is_open()) {
        std::cerr << "Error opening file: " << file << std::endl;
        return 1;
    }

    // decompress file
    return 0;
}


}