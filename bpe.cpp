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
void buffer_generate_with_header(std::vector<unsigned char>& buffer, const std::string& head, std::unordered_map<std::string, int>& vocab, const std::string& content){
    buffer.clear();
    for(char c : head){
        buffer.push_back(c);
    }
    std::string bormat_result = bormat(vocab.size(), 32);
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
    for(auto _vocab : vocab){
        int seql_bit = _vocab.first.length();
        bormat_result += bormat(seql_bit, 16);
        bormat_result += bormat(_vocab.first, alignment_up(seql_bit));
        bormat_result += bormat(_vocab.second, 16);
    }
    bormat_result += bormat(content.length(), 32);
    bormat_result += content;
    if (bormat_result.size() % 2 != 0) {
        bormat_result += '0';
    }
    for (size_t i = 0; i < bormat_result.size(); i += 2) {
        std::string byteString = bormat_result.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        buffer.push_back(byte);
    }
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

void HuffmanCompressor::parse_file(const std::string& filename){
    vocab.clear();
    size_t offset = 0;
    std::ifstream file(filename, std::ios::binary);
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string signature = read_chars(buffer, offset, 3);
    if(signature != "HFC"){
        throw std::runtime_error("Unknown file");
    }
    int vocab_len = read_ints(buffer, offset, 4);
    for(int i = 0; i < vocab_len; i++){
        int seql_bit = read_ints(buffer, offset, 2);
        std::string seq = read_chars(buffer, offset, alignment_up(seql_bit)/8);
        int freq = read_ints(buffer, offset, 2);
        vocab[seq] = freq;
    }
    int content_len = read_ints(buffer, offset, 4);
    std::string content = read_chars(buffer, offset, content_len);

    if(content_len %2 != 0){
        
    }

}



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
        size = bin.length() / 6;
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

    // 写入压缩文件
    std::ofstream out(output_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return;
    }
    saveTree(root, out);
    out.put('#'); // Tree and data separator
    out.close();

    // 将编码后的数据以二进制形式写入文件
    std::vector<unsigned char> binary_data;
    std::bitset<8> bits;
    int bitIndex = 0;
    for (char bit : result) {
        bits[bitIndex++] = bit - '0';
        if (bitIndex == 8) {
            binary_data.push_back(static_cast<unsigned char>(bits.to_ulong()));
            bits.reset();
            bitIndex = 0;
        }
    }
    if (bitIndex > 0) {
        binary_data.push_back(static_cast<unsigned char>(bits.to_ulong()));
    }
    out.write(reinterpret_cast<const char*>(binary_data.data()), binary_data.size());
    out.close();
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