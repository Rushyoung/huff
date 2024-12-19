#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <bitset>
#include <memory>
#include <set>
#include <exception>


std::string encoding(const std::string& str) {
    std::string result;
    for (unsigned char c : str) {
        std::bitset<8> bits(c);
        result += bits.to_string();
    }
    return result;
}

std::string decoding(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); i += 8) {
        std::bitset<8> bits(str.substr(i, 8));
        result += static_cast<char>(bits.to_ulong());
    }
    return result;
}

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};


class Node{
public:
    std::shared_ptr<Node> left, right;
    int freq;
    std::string seq;
    std::string code;
    Node(int freq, const std::string& seq) : freq(freq), seq(seq), left(nullptr), right(nullptr), code(""){}
};

class BPE{
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
public:
    std::unordered_map<std::string, int> vocab;
    BPE(const std::string& input, int batch):bin(input){
        for(size_t i = 0; i < bin.size(); i+= batch){
            bin_list.push_back(bin.substr(i, batch));
        }
        assert(bin_list.size() * batch == bin.size());
    }
    void train(int min_freq, int size = -1);
    std::shared_ptr<Node> build();
};