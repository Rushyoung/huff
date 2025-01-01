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

std::string encoding(const std::string& str);
std::string decoding(const std::string& str);

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

class Node {
public:
    std::shared_ptr<Node> left, right;
    int freq;
    std::string seq;
    std::string code;
    Node(int freq, const std::string& seq) : left(nullptr), right(nullptr), freq(freq), seq(seq), code("") {}
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
    std::string content;
    static void saveTree(const std::shared_ptr<Node>& root, std::ofstream& out);
    static std::shared_ptr<Node> loadTree(std::ifstream& in);

public:
    void file_generate_with_header(const std::string& outpath, const std::string& head, std::unordered_map<std::string, int>& vocab);
    std::string parse_file(const std::string& filename);
    static void printTree(const std::shared_ptr<Node>& root, int depth = 0);
    std::unordered_map<std::string, int> vocab;
    HuffmanCompressor(const std::string& input, int batch);
    void train(int min_freq, int size = -1);
    std::shared_ptr<Node> build();
    static void huffman_generate(Node& root, std::string code = "");
    static std::unordered_map<std::string, std::string> huffman_mapping(Node& n);
    static void compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq);
    static void decompress_file(const std::string& input_file, const std::string& output_file);
};