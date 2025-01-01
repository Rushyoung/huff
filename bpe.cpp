#include "bpe.hpp"
#include <iomanip>
#include <sstream>

std::string encoding(const std::string& str) {
    std::string result;
    for (unsigned char c : str) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        result += ss.str();
    }
    return result;
}

std::string decoding(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); i += 2) {
        std::string byteString = str.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        result += byte;
    }
    return result;
}


//只给十六进制补位
std::string bormat(const std::string& value, int bitnum) {
    std::stringstream ss;
    ss << std::hex << std::setw(bitnum / 4) << std::setfill('0') << value;
    return ss.str();
}

std::string bormat(int value, int bitnum) {
    std::stringstream ss;
    ss << std::hex << std::setw(bitnum / 4) << std::setfill('0') << value;
    return ss.str();
}



inline int calc_bit(int num){
    int bitWidth = 0;
    while (num > 0) {
        num >>= 1;
        ++bitWidth;
    }
    return bitWidth;
}

inline int alignment_up(int num){
    if(num % 8 == 0){return num;}
    else{return (num / 8 + 1) * 8;}
}

inline int alignment_down(int num){
    if(num % 8 == 0){return num;}
    else{return (num / 8 - 1) * 8;}
}

// 文件结构(弃用)
// +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
// | 文件签名 (3字节) | 词典数量 (32位) | 最大频率占的bit (32位) | 最长键值bit数 (32bit int)| 词汇频率 (占value_bit位 int) | seql_bit(占最大键值位) | 词汇键值 (seql_bit位) | 内容长度 (32位) | 内容 (数据) 
// +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+

// 一个hex = 4bit   1 unsigned char = 8bit 
// +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
// | 文件签名 (3字节) | 词典数量 (32bit) |  seql_bit (占16bit) | 词汇键值 向上对齐到 8n bit (有效内容seql_bit位) | 词汇频率 (占 16bit int) | 内容长度 (32位) | 内容 (数据) 
// +----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
// content : hex
void HuffmanCompressor::file_generate_with_header(const std::string& outpath, const std::string& head, std::unordered_map<std::string, int>& vocab){
    std::ofstream out(outpath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file");
    }
    out.write(head.c_str(), 3);
    uint32_t vocab_size = vocab.size();
    out.write(reinterpret_cast<const char*>(&vocab_size), sizeof(vocab_size));
    // std::string bormat_result = bormat(vocab.size(), 32);
    // int max_value = std::max_element(vocab.begin(), vocab.end(),
    //     [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    //         return a.second < b.second;
    //     })->second;
    // int max_seq_len = std::max_element(vocab.begin(), vocab.end(),
    //     [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
    //         return a.first.length() < b.first.length();
    //     })->first.length();
    // int value_bit = calc_bit(max_value);
    // int seql_bit = calc_bit(max_seq_len);
    // bormat_result += bormat(value_bit, 32);
    // bormat_result += bormat(seql_bit, 32);
    for(const auto& [seq, freq] : vocab){
        uint64_t key = 0;
        uint16_t seql_bit = seq.length();
        out.write(reinterpret_cast<const char*>(seql_bit), sizeof(seql_bit));
        for (char c : seq) {
            key = (key << 4) | (c & 0xF);
        }
        int byte_length = (seq.length() * 4 + 7) / 8; // 向上取整
        for (int i = byte_length - 1; i >= 0; --i) {
            unsigned char byte = (key >> (i * 8)) & 0xFF;
            out.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
        }
        out.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
        // bormat_result += bormat(seql_bit, 16);
        // bormat_result += bormat(_vocab.first, alignment_up(seql_bit));
        // bormat_result += bormat(_vocab.second, 16);
        
    }
    uint32_t content_size = content.length();
    out.write(reinterpret_cast<const char*>(&content_size), sizeof(content_size));
    // bormat_result += bormat(content.length(), 32);
    // bormat_result += content;
    std::vector<unsigned char> buffer;
    if (content.size() % 2 != 0) {
        content += '0';
    }
    for (size_t i = 0; i < content.size(); i += 2) {
        std::string byteString = content.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        buffer.push_back(byte);
    }
    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    out.close();
}

std::string read_chars(const std::vector<unsigned char>& buffer, size_t& offset, int charnum) {
    std::string result;
    for (int i = 0; i < charnum; ++i) {
        result += buffer[offset++];
    }
    return result;
}

int read_ints(const std::vector<unsigned char>& buffer, size_t& offset, int charnum) {
    int result = 0;
    for (int i = 0; i < charnum; ++i) {
        result = (result << 8) | buffer[offset + i];
    }
    offset += charnum;
    return result;
}

// 读取文件内容到缓冲区
std::vector<unsigned char> readFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Error opening input file");
    }
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// 从缓冲区读取指定类型的数据
template <typename T>
T readData(const std::vector<unsigned char>& buffer, size_t& offset) {
    T value;
    memcpy(&value, buffer.data() + offset, sizeof(T));
    offset += sizeof(T);
    return value;
}

// 解析文件并还原 vocab 和 content
std::string HuffmanCompressor::parse_file(const std::string& filename) {
    std::vector<unsigned char> buffer = readFile(filename);
    size_t offset = 0;

    // 读取文件签名
    std::string signature(buffer.begin(), buffer.begin() + 3);
    offset += 3;
    if (signature != "HFC") {
        throw std::runtime_error("Invalid file signature");
    }

    // 读取词典数量
    uint32_t vocab_size = readData<uint32_t>(buffer, offset);

    // 读取词典内容
    for (uint32_t i = 0; i < vocab_size; ++i) {
        uint16_t seql_bit = readData<uint16_t>(buffer, offset);
        int byte_length = (seql_bit * 4 + 7) / 8; // 向上取整
        uint64_t key = 0;
        for (int j = 0; j < byte_length; ++j) {
            key = (key << 8) | buffer[offset++];
        }
        std::string seq;
        for (int j = seql_bit - 1; j >= 0; --j) {
            char c = (key >> (j * 4)) & 0xF;
            seq += c;
        }
        int freq = readData<uint16_t>(buffer, offset);
        vocab[seq] = freq;
    }

    // 读取内容长度
    uint32_t content_size = readData<uint32_t>(buffer, offset);

    // 读取内容
    content.clear();
    for (uint32_t i = 0; i < content_size; ++i) {
        unsigned char byte = buffer[offset++];
        content += byte;
    }
}

// void HuffmanCompressor::parse_file(const std::string& filename){
//     vocab.clear();
//     size_t offset = 0;
//     std::ifstream file(filename, std::ios::binary);
//     std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     std::string signature = read_chars(buffer, offset, 3);
//     if(signature != "HFC"){
//         throw std::runtime_error("Unknown file");
//     }
//     int vocab_len = read_ints(buffer, offset, 4);
//     for(int i = 0; i < vocab_len; i++){
//         int seql_bit = read_ints(buffer, offset, 2);
//         std::string seq = read_chars(buffer, offset, alignment_up(seql_bit)/8);
//         int freq = read_ints(buffer, offset, 2);
//         vocab[seq] = freq;
//     }
//     int content_len = read_ints(buffer, offset, 4);
//     std::string content = read_chars(buffer, offset, content_len);

//     if(content_len %2 != 0){

//     }

// }



// 定义静态成员
std::unordered_map<std::string, std::string> HuffmanCompressor::mapping;

HuffmanCompressor::HuffmanCompressor(const std::string& input, int batch) : bin(input) {
    // 初始化 bin_list
    for (size_t i = 0; i < bin.size(); i += batch) {
        bin_list.push_back(bin.substr(i, batch));
    }
}
void HuffmanCompressor::printTree(const std::shared_ptr<Node>& root, int depth) {
    if (!root) {
        return;
    }

    // 打印当前节点的信息
    std::cout << std::setw(depth * 4) << "" // 缩进
              << "Freq: " << root->freq
              << ", Seq: " << root->seq
              << ", Code: " << root->code << std::endl;

    // 递归打印左子树和右子树
    printTree(root->left, depth + 1);
    printTree(root->right, depth + 1);
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
        size = bin.length() / 4;//change
    }
    if (size <= 0) {
        return;
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
        vocab[seq] += 1;
    }
    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto& [seq, freq] : vocab) {
        nodes.push_back(std::make_shared<Node>(freq, seq));
    }
    while (nodes.size() > 1) {
        std::sort(nodes.begin(), nodes.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
            return a->freq * a->seq.length() < b->freq * a->seq.length();
        });//exp
        auto left = nodes.front();
        nodes.erase(nodes.begin());
        auto right = nodes.front();
        nodes.erase(nodes.begin());
        auto parent = std::make_shared<Node>(left->freq + right->freq, "");
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
    if (root.left != nullptr) {
        huffman_generate(*root.left, code + "0");
    }
    if (root.right != nullptr) {
        huffman_generate(*root.right, code + "1");
    }
}

std::unordered_map<std::string, std::string> HuffmanCompressor::huffman_mapping(Node& n) {
    if (n.left == nullptr && n.right == nullptr) {
        mapping[n.seq] = n.code;
    } else {
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

void HuffmanCompressor::saveTree(const std::shared_ptr<Node>& root, std::ofstream& out) {
    if (!root) {
        out.put('#');
        return;
    }
    if (root->seq != "") {
        out.put('1');
        out.write(root->seq.c_str(), root->seq.size());
    } else {
        out.put('0');
    }
    saveTree(root->left, out);
    saveTree(root->right, out);
}

std::shared_ptr<Node> HuffmanCompressor::loadTree(std::ifstream& in) {
    char ch;
    in.get(ch);
    if (ch == '#') return nullptr;

    if (ch == '1') {
        char seq;
        in.read(&seq, 1);
        auto node = std::make_shared<Node>(0, std::string(1, seq));
        node->left = loadTree(in);
        node->right = loadTree(in);
        return node;
    } else {
        auto node = std::make_shared<Node>(0, "");
        node->left = loadTree(in);
        node->right = loadTree(in);
        return node;
    }
}

void HuffmanCompressor::compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq) {
    // 读取输入文件内容
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string input_str = buffer.str();
    in.close();

    // 编码
    std::string bin_str = encoding(input_str);

    // 训练
    HuffmanCompressor bpe(bin_str, batch);
    bpe.train(min_freq);
    std::shared_ptr<Node> root = bpe.build();

    // 生成哈夫曼编码
    huffman_generate(*root);
    std::unordered_map<std::string, std::string> mapping = huffman_mapping(*root);

    printTree(root);

    // 压缩
    std::string result;
    for (const auto& b : bpe.bin_list) {
        result += mapping[b];
    }

    // 使用 file_generate_with_header 函数保存文件
    bpe.file_generate_with_header(output_file, "HFC", bpe.vocab);
}

void HuffmanCompressor::decompress_file(const std::string& input_file, const std::string& output_file) {
    // 读取压缩文件内容
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return;
    }

    // 读取哈夫曼树
    std::shared_ptr<Node> root = loadTree(in);
    huffman_generate(*root);
    std::unordered_map<std::string, std::string> mapping = huffman_mapping(*root);

    // 读取压缩数据
    std::vector<unsigned char> binary_data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    // 将二进制数据转换回编码字符串
    std::string encoded_str;
    for (unsigned char c : binary_data) {
        std::bitset<8> bits(c);
        encoded_str += bits.to_string();
    }

    // 解码
    std::string decoded_bin_str;
    size_t i = 0;
    while (i < encoded_str.size()) {
        bool matched = false;
        for (const auto& pair : mapping) {
            if (encoded_str.substr(i, pair.second.size()) == pair.second) {
                decoded_bin_str += pair.first;
                i += pair.second.size();
                matched = true;
                break;
            }
        }
        if (!matched) {
            std::cerr << "Decoding error: no matching code found" << std::endl;
            return;
        }
    }
    std::string decoded_str = decoding(decoded_bin_str);

    // 写入解压缩文件
    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return;
    }
    out << decoded_str;
    out.close();
}