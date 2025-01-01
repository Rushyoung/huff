#ifndef FILE_HUFFMAN_HPP
#define FILE_HUFFMAN_HPP

#include <vector>
#include <string>
#include <cstdint>

struct HuffmanFile {
    char signature[3];
    uint32_t dict_size;
    uint16_t seql_bit;
    std::vector<uint64_t> vocab_keys;
    std::vector<uint16_t> vocab_freqs;
    uint32_t content_length;
    std::string content;
};

class FileHuffman {
public:
    static void generateFile(const std::string& output_path, 
                           const std::vector<std::pair<std::string, uint16_t>>& vocab);
                           
    static HuffmanFile parseFile(const std::string& input_path);
};

#endif // FILE_HUFFMAN_HPP
