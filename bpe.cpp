#include "bpe.hpp"


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

HuffmanCompressor::HuffmanCompressor(const std::string& input, int batch) : bin(input) {
    for (size_t i = 0; i < bin.size(); i += batch) {
        bin_list.push_back(bin.substr(i, batch));
    }
    assert(bin_list.size() * batch == bin.size());
}

void HuffmanCompressor::deal_vocab() {
    for (const auto& seq : bin_list) {
        vocab[seq]++;
    }
}

void HuffmanCompressor::deal_token() {
    token = std::unordered_map<std::string, int>(vocab);
}

void HuffmanCompressor::deal_pairs() {
    pairs.clear();
    pair_pos.clear();
    for (size_t i = 1; i < bin_list.size(); ++i) {
        std::pair<std::string, std::string> pair = {bin_list[i - 1], bin_list[i]};
        pairs[pair]++;
        pair_pos[pair].push_back(i - 1);
    }
}

void HuffmanCompressor::merge_vocab(int min_freq) {
    auto ori_list = std::vector<std::string>(bin_list);
    auto best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    })->first;

    while (blacklist.find(best_pair) != blacklist.end()) {
        pairs.erase(best_pair);
        best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->first;
    }

    if (pairs[best_pair] <= min_freq) {
        return;
    }

    std::string best_bin = best_pair.first + best_pair.second;
    size_t length = bin_list.size();
    size_t offset = 0;
    for (int pos : pair_pos[best_pair]) {
        size_t idx = pos - offset;
        if (idx < 0 || idx >= length) {
            continue;
        }
        if (bin_list[idx] != best_pair.first || bin_list[idx + 1] != best_pair.second) {
            continue;
        }
        bin_list[idx] = best_bin;
        bin_list.erase(bin_list.begin() + idx + 1);
        offset++;
    }
    int freq = pairs[best_pair];
    token[best_bin] = freq;
    token[best_pair.first] -= freq;
    token[best_pair.second] -= freq;
    auto common_freq = [min_freq](int a) {
        return a <= 0 || a > min_freq;
    };
    if (!common_freq(token[best_pair.first]) || !common_freq(token[best_pair.second])) {
        bin_list = ori_list;
        token[best_bin] = 0;
        token[best_pair.first] += freq;
        token[best_pair.second] += freq;
        blacklist.insert(best_pair);
    }
}

void HuffmanCompressor::train(int min_freq, int size) {
    if (size == -1) {
        size = bin.length() / 6;
    }
    if (size <= 0) {
        throw std::invalid_argument("Size must be greater than 0");
    }
    deal_vocab();
    deal_token();
    for (int i = 0; i < size; i++) {
        deal_pairs();
        merge_vocab(min_freq);
    }
}

std::shared_ptr<Node> HuffmanCompressor::build() {
    vocab.clear();
    for (const auto& seq : bin_list) {
        vocab[seq]++;
    }
    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto& [seq, freq] : vocab) {
        nodes.push_back(std::make_shared<Node>(freq, seq));
    }
    while (nodes.size() > 1) {
        std::sort(nodes.begin(), nodes.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
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

void HuffmanCompressor::huffman_generate(Node& root, std::string code) {
    if (root.left == nullptr && root.right == nullptr) {
        root.code = code;
        return;
    }
    if (root.left != nullptr) { huffman_generate(*root.left, code + "0"); }
    if (root.right != nullptr) { huffman_generate(*root.right, code + "1"); }
}

std::unordered_map<std::string, std::string> HuffmanCompressor::huffman_mapping(Node& n) {
    std::unordered_map<std::string, std::string> mapping;
    if (n.left == nullptr && n.right == nullptr) {
        mapping[n.seq] = n.code;
    }
    else {
        if (n.left) {
            auto left_mapping = huffman_mapping(*n.left);
            mapping.insert(left_mapping.begin(), left_mapping.end());
        }
        if (n.right) {
            auto right_mapping = huffman_mapping(*n.right);
            mapping.insert(right_mapping.begin(), right_mapping.end());
        }
    }
    return mapping;
}

void HuffmanCompressor::compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq) {
    // 读取输入文件内容
    std::ifstream infile(input_file);
    if (!infile) {
        throw std::runtime_error("Failed to open input file");
    }
    std::stringstream buffer;
    buffer << infile.rdbuf();
    std::string input_str = buffer.str();
    infile.close();

    // 编码
    std::string bin_str = encoding(input_str);

    // 训练
    HuffmanCompressor compressor(bin_str, batch);
    compressor.train(min_freq);
    std::shared_ptr<Node> root = compressor.build();

    // 生成哈夫曼编码
    HuffmanCompressor::huffman_generate(*root);
    std::unordered_map<std::string, std::string> mapping = HuffmanCompressor::huffman_mapping(*root);

    // 压缩
    std::string compressed_bits;
    for (const auto& b : compressor.bin_list) {
        compressed_bits += mapping[b];
    }

    // 将哈夫曼编码映射和压缩后的数据写入文件
    std::ofstream outfile(output_file, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Failed to open output file");
    }

    // 写入映射大小
    size_t map_size = mapping.size();
    outfile.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

    // 写入映射
    for (const auto& pair : mapping) {
        size_t key_size = pair.first.size();
        size_t value_size = pair.second.size();
        outfile.write(reinterpret_cast<const char*>(&key_size), sizeof(key_size));
        outfile.write(pair.first.data(), key_size);
        outfile.write(reinterpret_cast<const char*>(&value_size), sizeof(value_size));
        outfile.write(pair.second.data(), value_size);
    }

    // 写入压缩数据
    size_t compressed_size = compressed_bits.size();
    outfile.write(reinterpret_cast<const char*>(&compressed_size), sizeof(compressed_size));
    for (size_t i = 0; i < compressed_bits.size(); i += 8) {
        std::bitset<8> bits(compressed_bits.substr(i, 8));
        unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
        outfile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
    }

    outfile.close();
}

void HuffmanCompressor::decompress_file(const std::string& input_file, const std::string& output_file) {
    // 读取压缩文件内容
    std::ifstream infile(input_file, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Failed to open input file");
    }

    // 读取映射大小
    size_t map_size;
    infile.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));

    // 读取映射
    std::unordered_map<std::string, std::string> mapping;
    for (size_t i = 0; i < map_size; ++i) {
        size_t key_size, value_size;
        infile.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        std::string key(key_size, ' ');
        infile.read(&key[0], key_size);
        infile.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        std::string value(value_size, ' ');
        infile.read(&value[0], value_size);
        mapping[key] = value;
    }

    // 读取压缩数据大小
    size_t compressed_size;
    infile.read(reinterpret_cast<char*>(&compressed_size), sizeof(compressed_size));

    // 读取压缩数据
    std::string compressed_bits;
    for (size_t i = 0; i < compressed_size; ++i) {
        unsigned char byte;
        infile.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        std::bitset<8> bits(byte);
        compressed_bits += bits.to_string();
    }

    infile.close();

    // 解码
    std::string decoded_bin_str;
    for (size_t i = 0; i < compressed_bits.size(); ) {
        for (const auto& pair : mapping) {
            if (compressed_bits.substr(i, pair.second.size()) == pair.second) {
                decoded_bin_str += pair.first;
                i += pair.second.size();
                break;
            }
        }
    }
    std::string decoded_str = decoding(decoded_bin_str);

    // 写入解压缩文件
    std::ofstream outfile(output_file);
    if (!outfile) {
        throw std::runtime_error("Failed to open output file");
    }
    outfile << decoded_str;
    outfile.close();
}