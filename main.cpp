#include "file_huffman.hpp"

int main(){
    file_huffman::compressFile("z24.png", "3.bin");
    file_huffman::decompressFile("3.bin", "4.png");
//    file_huffman::decompressFile("2.bin", "3.txt");
}

