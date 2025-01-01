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

std::vector<uint8_t> encoding(const std::string& str);
std::string decoding(const std::string& str);

namespace std {
    template<>
    struct hash<vector<uint8_t>> {
        size_t operator()(const vector<uint8_t>& v) const {
            size_t hash = v.size();
            for(auto& i : v) {
                hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
}
struct pair_hash {
    template<typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ (hash2 << 1);
    }
};




// class Node {
// public:
//     std::shared_ptr<Node> left, right;
//     int freq;
//     std::string seq;
//     std::string code;
//     Node(int freq, const std::string& seq) : left(nullptr), right(nullptr), freq(freq), seq(seq), code("") {}
// };
class Node {
public:
    std::shared_ptr<Node> left, right;
    int freq;
    std::vector<uint8_t> seq;
    std::string code;
    Node(int freq, const std::vector<uint8_t>& seq) 
        : left(nullptr), right(nullptr), freq(freq), seq(seq), code("") {}
};
class HuffmanCompressor {
private:
    void deal_vocab();
    void deal_token();
    void deal_pairs();
    void merge_vocab(int min_freq);
    std::vector<uint8_t> bin;
    std::vector<std::vector<uint8_t>> bin_list;
    std::unordered_map<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>, int, pair_hash> pairs;
    std::unordered_map<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>, std::vector<int>, pair_hash> pair_pos;
    std::unordered_map<std::vector<uint8_t>, int> token;
    std::set<std::pair<std::vector<uint8_t>, std::vector<uint8_t>>> blacklist;
    static std::unordered_map<std::vector<uint8_t>, std::string> mapping;
    std::vector<uint8_t> content;
    std::unordered_map<std::vector<uint8_t>, int> vocab;

public:
    void file_generate_with_header(const std::string& outpath, const std::string& head);
    void parse_file(const std::string& filename);
    static void printTree(const std::shared_ptr<Node>& root, int depth = 0);
    HuffmanCompressor(const std::string& input, int batch);
    void train(int min_freq, int size = -1);
    std::shared_ptr<Node> build();
    static void huffman_generate(Node& root, std::string code = "");
    static std::unordered_map<std::vector<uint8_t>, std::string> huffman_mapping(Node& n);
    static void compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq);
    static void decompress_file(const std::string& input_file, const std::string& output_file);
};