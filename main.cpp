// #include "file_huffman.hpp"
//
// int main(){
//     file_huffman::compressFile("z24.png", "3.bin");
//     file_huffman::decompressFile("3.bin", "4.png");
// //    file_huffman::compressFile("1.txt", "2.bin");
// //    file_huffman::decompressFile("2.bin", "3.txt");
// //    file_huffman::compressFile("huff.exe", "huff.bin");
// //    file_huffman::decompressFile("huff.bin", "huff2.exe");
// }
//


#include "bpe.hpp"
#include <iostream>


int main() {
    std::string input_file = "1.txt";
    std::string compressed_file = "compressed.bin";
    std::string decompressed_file = "decompressed.txt";

    try {
        // 压缩文件
        HuffmanCompressor::compress_file(input_file, compressed_file, 4, 2);
        std::cout << "File compressed successfully." << std::endl;

        // 解压缩文件
        // HuffmanCompressor::decompress_file(compressed_file, decompressed_file);
        // std::cout << "File decompressed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}