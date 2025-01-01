#include "file_huffman.hpp"
#include <fstream>
#include <bitset>
#include <algorithm>

void FileHuffman::generateFile(const std::string& output_path, 
                             const std::vector<std::pair<std::string, uint16_t>>& vocab) {
    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file");
    }

    // Write signature
    out.write("HUF", 3);

    // Write dictionary size
    uint32_t dict_size = static_cast<uint32_t>(vocab.size());
    out.write(static_cast<char*>(static_cast<void*>(&dict_size)), sizeof(dict_size));

    // Calculate and write seql_bit
    uint16_t max_seq_len = 0;
    for (const auto& [seq, freq] : vocab) {
        max_seq_len = std::max(max_seq_len, static_cast<uint16_t>(seq.size() * 4));
    }
    out.write(static_cast<char*>(static_cast<void*>(&max_seq_len)), sizeof(max_seq_len));

    // Write vocab keys and frequencies
    for (const auto& [seq, freq] : vocab) {
        uint64_t key = 0;
        for (char c : seq) {
            key = (key << 4) | (c & 0xF);
        }
        // Align to 8n bits
        key <<= (64 - max_seq_len) % 8;
        out.write(static_cast<char*>(static_cast<void*>(&key)), sizeof(key));
        out.write(static_cast<char*>(static_cast<void*>(&freq)), sizeof(freq));
    }

    // TODO: Implement Huffman encoding and write content
    // Placeholder for content length and content
    uint32_t content_length = 0;
    out.write(static_cast<char*>(static_cast<void*>(&content_length)), sizeof(content_length));
}

HuffmanFile FileHuffman::parseFile(const std::string& input_path) {
    std::ifstream in(input_path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open input file");
    }

    HuffmanFile file;

    // Read signature
    in.read(file.signature, 3);
    if (std::string(file.signature, 3) != "HUF") {
        throw std::runtime_error("Invalid file signature");
    }

    // Read dictionary size
    in.read(static_cast<char*>(static_cast<void*>(&file.dict_size)), sizeof(file.dict_size));

    // Read seql_bit
    in.read(static_cast<char*>(static_cast<void*>(&file.seql_bit)), sizeof(file.seql_bit));

    // Read vocab keys and frequencies
    file.vocab_keys.resize(file.dict_size);
    file.vocab_freqs.resize(file.dict_size);
    for (uint32_t i = 0; i < file.dict_size; ++i) {
        in.read(static_cast<char*>(static_cast<void*>(&file.vocab_keys[i])), sizeof(uint64_t));
        in.read(static_cast<char*>(static_cast<void*>(&file.vocab_freqs[i])), sizeof(uint16_t));
    }

    // Read content length and content
    in.read(static_cast<char*>(static_cast<void*>(&file.content_length)), sizeof(file.content_length));
    file.content.resize(file.content_length);
    in.read(file.content.data(), file.content_length);

    return file;
}
