#include "bpe.hpp"
#include <iostream>

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


void BPE::deal_vocab(){
    for(const auto& seq : bin_list){
        vocab[seq]++;
    }
}

void BPE::deal_token(){
    token = std::unordered_map<std::string, int>(vocab);
}

void BPE::deal_pairs(){
    pairs.clear();
    pair_pos.clear();
    for(size_t i = 1; i < bin_list.size(); ++i){
        std::pair<std::string, std::string> pair = {bin_list[i-1], bin_list[i]};
        pairs[pair]++;
        pair_pos[pair].push_back(i-1);
    }
}

void BPE:: merge_vocab(int min_freq){
    auto ori_list = bin_list;
    auto best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b){
        return a.second < b.second;
    })->first;
    
    while(blacklist.find(best_pair) != blacklist.end()){
        pairs.erase(best_pair);
        best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b){
            return a.second < b.second;
        })->first;
    }

    if(pairs[best_pair] <= min_freq){
        return;
    }

    std::string best_bin = best_pair.first + best_pair.second;
    size_t length = bin_list.size();
    size_t offset = 0;
    for(int pos : pair_pos[best_pair]){
        size_t idx = pos - offset;
        if(idx < 0 || idx >= length){
            continue;
        }
        if(bin_list[idx] != best_pair.first || bin_list[idx + 1] != best_pair.second){
            continue;
        }
        bin_list[idx] = best_bin;
        bin_list.erase(bin_list.begin() + idx +1);
        offset++;
    }
    int freq = pairs[best_pair];
    token[best_bin] = freq;
    token[best_pair.first] -= freq;
    token[best_pair.second] -= freq;
    auto common_freq = [min_freq](int a){
        return a <= 0 || a > min_freq;
    };
    if(!common_freq(token[best_pair.first]) || ! common_freq(token[best_pair.second])){
        bin_list = ori_list;
        token[best_bin] = 0;
        token[best_pair.first] += freq;
        token[best_pair.second] += freq;
        blacklist.insert(best_pair);
    }
}

void BPE::train(int min_freq, int size){
    //TODO:max_vocab
    if(size == -1){
        size = bin.length() / 6;
    }
    if(size <= 0){
        // throw std::bad_exception();
        return;
    }
    if(size < bin.length()){
        //throw std::bad_exception();
        return;
    }
    deal_vocab();
    deal_token();
    for(int i = 0; i < size; i++){
        deal_pairs();
        merge_vocab(min_freq);
    }
}

std::shared_ptr<Node> BPE::build(){
    vocab.clear();
    for(const auto& seq : bin_list){
        vocab[seq] += 1;
    }
    std::vector<std::shared_ptr<Node>> nodes;
    for(const auto& [seq, freq] : vocab){
        nodes.push_back(std::make_shared<Node>(freq, seq));
    }
    while(nodes.size() > 1){
        std::sort(nodes.begin(), nodes.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b){
            return a->freq < b->freq;
        });
        auto left = nodes.front();
        nodes.erase(nodes.begin());
        auto right = nodes.front();
        nodes.erase(nodes.begin());
        auto parent = std::make_shared<Node>(left->freq + right->freq, "not leaf");
        parent->left = left;
        parent->right = right;
        nodes.push_back(parent);
    }
    return nodes.front();
}


