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

    // write filename to .hfc.tmp, use text mode
    std::ofstream tmp(".hfc.tmp", std::ios::binary);
    if(not tmp.is_open()) {
        std::cerr << "Error opening file: .hfc.tmp" << std::endl;
        return 1;
    }
    tmp << file;
    tmp.close();

    // Run JSRT to generate the dictionary
    JS_Run();

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