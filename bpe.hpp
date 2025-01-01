// 移除vector<uint8_t>的hash实现
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <memory>
#include <set>
#include <fstream>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <functional>

std::string encoding(const std::string& str);
std::string decoding(const std::string& str);

struct pair_hash {
    template<typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ (hash2 << 1);
    }
};

class Node {
public:
    std::shared_ptr<Node> left, right;
    int freq;
    std::string seq;
    std::string code;
    Node(int freq, const std::string& seq) 
        : left(nullptr), right(nullptr), freq(freq), seq(seq), code("") {}
};

class HuffmanCompressor {
private:
    void deal_vocab();
    void deal_token();
    void deal_pairs();
    void merge_vocab(int min_freq);

    std::string bin;
    std::vector<std::string> bin_list;
    std::unordered_map<std::pair<std::string, std::string>, int, pair_hash> pairs;
    std::unordered_map<std::pair<std::string, std::string>, std::vector<int>, pair_hash> pair_pos;
    std::unordered_map<std::string, int> token;
    std::set<std::pair<std::string, std::string>> blacklist;
    static std::unordered_map<std::string, std::string> mapping;
    std::unordered_map<std::string, int> vocab;
    std::string compressed_hex;  // 压缩后的十六进制字符串
    uint8_t valid_bits; 

public:
    static std::vector<uint8_t> hex_string_to_bytes(const std::string& hex);
    static std::string bytes_to_hex_string(const std::vector<uint8_t>& bytes);

    static std::string hex_to_bits_aligned(const std::string& hex, uint8_t valid_bits);


    void file_generate_with_header(const std::string& outpath, const std::string& head);
    void parse_file(const std::string& filename);
    static void printTree(const std::shared_ptr<Node>& root, int depth = 0);
    HuffmanCompressor(const std::string& input, int batch);
    void train(int min_freq, int size = -1);
    std::shared_ptr<Node> build();
    static void huffman_generate(Node& root, std::string code = "");
    static std::unordered_map<std::string, std::string> huffman_mapping(Node& n);
    static void compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq);
    static void decompress_file(const std::string& input_file, const std::string& output_file);

    static std::string bits_to_hex(const std::string& bits);  // 位串转十六进制
    static std::string hex_to_bits(const std::string& hex);   // 十六进制转位串
    static std::vector<uint8_t> hex_to_bytes(const std::string& hex); // 最终写入时使用
};