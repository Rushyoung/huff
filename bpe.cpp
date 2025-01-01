#include "bpe.hpp"
#include <iomanip>
#include <sstream>
std::unordered_map<std::vector<uint8_t>, std::string> HuffmanCompressor::mapping;
// std::string encoding(const std::string& str) {
//     std::string result;
//     for (unsigned char c : str) {
//         std::stringstream ss;
//         ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
//         result += ss.str();
//     }
//     return result;
// }

std::vector<uint8_t> encoding(const std::string& str) {
    std::vector<uint8_t> result;
    result.reserve(str.length() * 2);
    for (unsigned char c : str) {
        result.push_back((c >> 4) & 0xF);  // 高4位
        result.push_back(c & 0xF);         // 低4位
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
// void HuffmanCompressor::file_generate_with_header(const std::string& outpath, const std::string& head, std::unordered_map<std::string, int>& vocab){
//     std::ofstream out(outpath, std::ios::binary);
//     if (!out) {
//         throw std::runtime_error("Failed to open output file");
//     }
//     out.write(head.c_str(), 3);
//     uint32_t vocab_size = vocab.size();
//     out.write(reinterpret_cast<const char*>(&vocab_size), sizeof(vocab_size));
//     // std::string bormat_result = bormat(vocab.size(), 32);
//     // int max_value = std::max_element(vocab.begin(), vocab.end(),
//     //     [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
//     //         return a.second < b.second;
//     //     })->second;
//     // int max_seq_len = std::max_element(vocab.begin(), vocab.end(),
//     //     [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
//     //         return a.first.length() < b.first.length();
//     //     })->first.length();
//     // int value_bit = calc_bit(max_value);
//     // int seql_bit = calc_bit(max_seq_len);
//     // bormat_result += bormat(value_bit, 32);
//     // bormat_result += bormat(seql_bit, 32);
//     for(const auto& [seq, freq] : vocab){
//         uint64_t key = 0;
//         uint16_t seql_bit = seq.length();
//         out.write(reinterpret_cast<const char*>(seql_bit), sizeof(seql_bit));
//         for (char c : seq) {
//             key = (key << 4) | (c & 0xF);
//         }
//         int byte_length = (seq.length() * 4 + 7) / 8; // 向上取整
//         for (int i = byte_length - 1; i >= 0; --i) {
//             unsigned char byte = (key >> (i * 8)) & 0xFF;
//             out.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
//         }
//         out.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
//         // bormat_result += bormat(seql_bit, 16);
//         // bormat_result += bormat(_vocab.first, alignment_up(seql_bit));
//         // bormat_result += bormat(_vocab.second, 16);
        
//     }
//     uint32_t content_size = content.length();
//     out.write(reinterpret_cast<const char*>(&content_size), sizeof(content_size));
//     // bormat_result += bormat(content.length(), 32);
//     // bormat_result += content;
//     std::vector<unsigned char> buffer;
//     if (content.size() % 2 != 0) {
//         content += '0';
//     }
//     for (size_t i = 0; i < content.size(); i += 2) {
//         std::string byteString = content.substr(i, 2);
//         unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
//         buffer.push_back(byte);
//     }
//     out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
//     out.close();
// }

void HuffmanCompressor::file_generate_with_header(const std::string& outpath, const std::string& head) {
    std::ofstream out(outpath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open output file");
    }

    // 1. 写入文件头 (3字节)
    out.write(head.c_str(), 3);

    // 2. 写入词典数量 (32bit)
    uint32_t vocab_size = vocab.size();
    out.write(reinterpret_cast<const char*>(&vocab_size), sizeof(vocab_size));

    // 3. 写入词典项
    for(const auto& [seq, freq] : vocab) {
        // 写入序列长度 (16bit) - 实际长度，不是bit长度
        uint16_t seq_size = seq.size();
        out.write(reinterpret_cast<const char*>(&seq_size), sizeof(seq_size));
        
        // 写入序列内容
        out.write(reinterpret_cast<const char*>(seq.data()), seq_size);
        
        // 写入频率 (16bit)
        uint16_t freq_val = static_cast<uint16_t>(freq);
        out.write(reinterpret_cast<const char*>(&freq_val), sizeof(freq_val));
    }

    // 4. 写入压缩内容
    // 写入内容大小 (32bit)
    uint32_t content_size = content.size();
    out.write(reinterpret_cast<const char*>(&content_size), sizeof(content_size));
    
    // 写入有效位数 (8bit)
    uint8_t valid_bits = (content_size > 0) ? 8 : 0;
    out.write(reinterpret_cast<const char*>(&valid_bits), sizeof(valid_bits));
    
    // 写入内容
    if (content_size > 0) {
        out.write(reinterpret_cast<const char*>(content.data()), content_size);
    }
    
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
// std::string HuffmanCompressor::parse_file(const std::string& filename) {
//     std::vector<unsigned char> buffer = readFile(filename);
//     size_t offset = 0;

//     // 读取文件签名
//     std::string signature(buffer.begin(), buffer.begin() + 3);
//     offset += 3;
//     if (signature != "HFC") {
//         throw std::runtime_error("Invalid file signature");
//     }

//     // 读取词典数量
//     uint32_t vocab_size = readData<uint32_t>(buffer, offset);

//     // 读取词典内容
//     for (uint32_t i = 0; i < vocab_size; ++i) {
//         uint16_t seql_bit = readData<uint16_t>(buffer, offset);
//         int byte_length = (seql_bit * 4 + 7) / 8; // 向上取整
//         uint64_t key = 0;
//         for (int j = 0; j < byte_length; ++j) {
//             key = (key << 8) | buffer[offset++];
//         }
//         std::string seq;
//         for (int j = seql_bit - 1; j >= 0; --j) {
//             char c = (key >> (j * 4)) & 0xF;
//             seq += c;
//         }
//         int freq = readData<uint16_t>(buffer, offset);
//         vocab[seq] = freq;
//     }

//     // 读取内容长度
//     uint32_t content_size = readData<uint32_t>(buffer, offset);

//     // 读取内容
//     content.clear();
//     for (uint32_t i = 0; i < content_size; ++i) {
//         unsigned char byte = buffer[offset++];
//         content += byte;
//     }
// }

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





// HuffmanCompressor::HuffmanCompressor(const std::string& input, int batch) : bin(input) {
//     // 初始化 bin_list
//     for (size_t i = 0; i < bin.size(); i += batch) {
//         bin_list.push_back(bin.substr(i, batch));
//     }
// }
HuffmanCompressor::HuffmanCompressor(const std::string& input, const int batch)
    : token(std::unordered_map<std::vector<uint8_t>, int>()),
      vocab(std::unordered_map<std::vector<uint8_t>, int>()) {
    bin = encoding(input);
    for (size_t i = 0; i < bin.size(); i += batch * 2) {
        std::vector<uint8_t> chunk;
        chunk.reserve(batch * 2);
        for (size_t j = 0; j < batch * 2 && i + j < bin.size(); ++j) {
            chunk.push_back(bin[i + j]);
        }
        bin_list.push_back(chunk);
    }
}
void HuffmanCompressor::printTree(const std::shared_ptr<Node>& root, int depth) {
    if (!root) {
        return;
    }

    std::cout << std::setw(depth * 4) << "" // 缩进
              << "Freq: " << root->freq
              << ", Seq: ";
    
    // 打印字节序列
    for (uint8_t byte : root->seq) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(byte) << " ";
    }
    std::cout << ", Code: " << root->code << std::endl;

    printTree(root->left, depth + 1);
    printTree(root->right, depth + 1);
}



void HuffmanCompressor::deal_vocab() {
    for (const auto& seq : bin_list) {
        vocab[seq]++;
    }
}

void HuffmanCompressor::deal_token() {
    token = vocab;
}

// void HuffmanCompressor::deal_pairs() {
//     pairs.clear();
//     pair_pos.clear();
//     for (size_t i = 1; i < bin_list.size(); ++i) {
//         std::pair<std::string, std::string> pair = {bin_list[i - 1], bin_list[i]};
//         pairs[pair]++;
//         pair_pos[pair].push_back(i - 1);
//     }
// }
void HuffmanCompressor::deal_pairs() {
    pairs.clear();
    pair_pos.clear();
    for (size_t i = 1; i < bin_list.size(); ++i) {
        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> pair = {bin_list[i - 1], bin_list[i]};
        pairs[pair]++;
        pair_pos[pair].push_back(i - 1);
    }
}

// void HuffmanCompressor::merge_vocab(int min_freq) {
//     auto ori_list = bin_list;
//     auto best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
//         return a.second < b.second;
//     })->first;

//     while (blacklist.find(best_pair) != blacklist.end()) {
//         pairs.erase(best_pair);
//         best_pair = std::max_element(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
//             return a.second < b.second;
//         })->first;
//     }

//     if (pairs[best_pair] <= min_freq) {
//         return;
//     }

//     std::string best_bin = best_pair.first + best_pair.second;
//     size_t length = bin_list.size();
//     size_t offset = 0;
//     for (int pos : pair_pos[best_pair]) {
//         size_t idx = pos - offset;
//         if (idx < 0 || idx >= length) {
//             continue;
//         }
//         if (bin_list[idx] != best_pair.first || bin_list[idx + 1] != best_pair.second) {
//             continue;
//         }
//         bin_list[idx] = best_bin;
//         bin_list.erase(bin_list.begin() + idx + 1);
//         offset++;
//     }
//     int freq = pairs[best_pair];
//     token[best_bin] = freq;
//     token[best_pair.first] -= freq;
//     token[best_pair.second] -= freq;
//     auto common_freq = [min_freq](int a) {
//         return a <= 0 || a > min_freq;
//     };
//     if (!common_freq(token[best_pair.first]) || !common_freq(token[best_pair.second])) {
//         bin_list = ori_list;
//         token[best_bin] = 0;
//         token[best_pair.first] += freq;
//         token[best_pair.second] += freq;
//         blacklist.insert(best_pair);
//     }
// }

void HuffmanCompressor::merge_vocab(int min_freq) {
    auto ori_list = bin_list;
    auto best_pair = std::max_element(pairs.begin(), pairs.end(), 
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->first;

    while (blacklist.find(best_pair) != blacklist.end()) {
        pairs.erase(best_pair);
        best_pair = std::max_element(pairs.begin(), pairs.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            })->first;
    }

    if (pairs[best_pair] <= min_freq) {
        return;
    }

    std::vector<uint8_t> best_bin;
    best_bin.reserve(best_pair.first.size() + best_pair.second.size());
    best_bin.insert(best_bin.end(), best_pair.first.begin(), best_pair.first.end());
    best_bin.insert(best_bin.end(), best_pair.second.begin(), best_pair.second.end());

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
}

void HuffmanCompressor::train(int min_freq, int size) {
    if (size == -1) {
        size = bin.size() / 128;//change
    }
    if (size <= 0) {
        return;
    }
    if(size > 256){
        size = 256;
    }
    deal_vocab();
    deal_token();
    for (int i = 0; i < size; i++) {
        deal_pairs();
        merge_vocab(min_freq);
    }
}

// std::shared_ptr<Node> HuffmanCompressor::build() {
//     vocab.clear();
//     for (const auto& seq : bin_list) {
//         vocab[seq] += 1;
//     }
//     std::vector<std::shared_ptr<Node>> nodes;
//     for (const auto& [seq, freq] : vocab) {
//         nodes.push_back(std::make_shared<Node>(freq, seq));
//     }
//     while (nodes.size() > 1) {
//         std::sort(nodes.begin(), nodes.end(), [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
//             return a->freq * a->seq.length() < b->freq * a->seq.length();
//         });//exp
//         auto left = nodes.front();
//         nodes.erase(nodes.begin());
//         auto right = nodes.front();
//         nodes.erase(nodes.begin());
//         auto parent = std::make_shared<Node>(left->freq + right->freq, "");
//         parent->left = left;
//         parent->right = right;
//         nodes.push_back(parent);
//     }
//     return nodes.front();
// }
std::shared_ptr<Node> HuffmanCompressor::build() {
    // vocab.clear();
    for (const auto& seq : bin_list) {
        vocab[seq] += 1;
    }

    std::vector<std::shared_ptr<Node>> nodes;
    for (const auto& [seq, freq] : vocab) {
        nodes.push_back(std::make_shared<Node>(freq, seq));
    }

    while (nodes.size() > 1) {
        std::sort(nodes.begin(), nodes.end(), 
            [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
                return a->freq * a->seq.size() < b->freq * b->seq.size();
            });
        
        auto left = nodes.front();
        nodes.erase(nodes.begin());
        auto right = nodes.front();
        nodes.erase(nodes.begin());

        std::vector<uint8_t> parent_seq;
        auto parent = std::make_shared<Node>(left->freq + right->freq, parent_seq);
        parent->left = left;
        parent->right = right;
        nodes.push_back(parent);
    }
    return nodes.front();
}

// void HuffmanCompressor::huffman_generate(Node& root, std::string code) {
//     if (root.left == nullptr && root.right == nullptr) {
//         root.code = code;
//         return;
//     }
//     if (root.left != nullptr) {
//         huffman_generate(*root.left, code + "0");
//     }
//     if (root.right != nullptr) {
//         huffman_generate(*root.right, code + "1");
//     }
// }

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



// std::unordered_map<std::string, std::string> HuffmanCompressor::huffman_mapping(Node& n) {
//     if (n.left == nullptr && n.right == nullptr) {
//         mapping[n.seq] = n.code;
//     } else {
//         if (n.left) {
//             auto left_mapping = huffman_mapping(*n.left);
//             mapping.insert(left_mapping.begin(), left_mapping.end());
//         }
//         if (n.right) {
//             auto right_mapping = huffman_mapping(*n.right);
//             mapping.insert(right_mapping.begin(), right_mapping.end());
//         }
//     }
//     return mapping;
// }

std::unordered_map<std::vector<uint8_t>, std::string> HuffmanCompressor::huffman_mapping(Node& n) {
    if (n.left == nullptr && n.right == nullptr) {
        mapping[n.seq] = n.code;
    } else {
        if (n.left) {
            huffman_mapping(*n.left);
        }
        if (n.right) {
            huffman_mapping(*n.right);
        }
    }
    return mapping;
}

// void HuffmanCompressor::saveTree(const std::shared_ptr<Node>& root, std::ofstream& out) {
//     if (!root) {
//         out.put('#');
//         return;
//     }
//     if (root->seq != "") {
//         out.put('1');
//         out.write(root->seq.c_str(), root->seq.size());
//     } else {
//         out.put('0');
//     }
//     saveTree(root->left, out);
//     saveTree(root->right, out);
// }

// std::shared_ptr<Node> HuffmanCompressor::loadTree(std::ifstream& in) {
//     char ch;
//     in.get(ch);
//     if (ch == '#') return nullptr;

//     if (ch == '1') {
//         char seq;
//         in.read(&seq, 1);
//         auto node = std::make_shared<Node>(0, std::string(1, seq));
//         node->left = loadTree(in);
//         node->right = loadTree(in);
//         return node;
//     } else {
//         auto node = std::make_shared<Node>(0, "");
//         node->left = loadTree(in);
//         node->right = loadTree(in);
//         return node;
//     }
// }

// void HuffmanCompressor::compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq) {
//     // 读取输入文件内容
//     std::ifstream in(input_file, std::ios::binary);
//     if (!in.is_open()) {
//         std::cerr << "Error opening input file" << std::endl;
//         return;
//     }

//     std::stringstream buffer;
//     buffer << in.rdbuf();
//     std::string input_str = buffer.str();
//     in.close();

//     // 编码
//     std::string bin_str = encoding(input_str);

//     // 训练
//     HuffmanCompressor bpe(bin_str, batch);
//     bpe.train(min_freq);
//     std::shared_ptr<Node> root = bpe.build();

//     // 生成哈夫曼编码
//     huffman_generate(*root);
//     std::unordered_map<std::string, std::string> mapping = huffman_mapping(*root);

//     printTree(root);

//     // 压缩
//     std::string result;
//     for (const auto& b : bpe.bin_list) {
//         result += mapping[b];
//     }

//     // 使用 file_generate_with_header 函数保存文件
//     bpe.file_generate_with_header(output_file, "HFC", bpe.vocab);
// }

// 修改压缩函数
void HuffmanCompressor::compress_file(const std::string& input_file, const std::string& output_file, int batch, int min_freq) {
    // 读取输入文件
    std::ifstream in(input_file, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Error opening input file");
    }

    std::vector<uint8_t> input_data((std::istreambuf_iterator<char>(in)), 
                                   std::istreambuf_iterator<char>());
    in.close();

    // 转换为十六进制并训练
    std::string input_str(input_data.begin(), input_data.end());
    HuffmanCompressor bpe(input_str, batch);
    bpe.train(min_freq);
    std::shared_ptr<Node> root = bpe.build();

    // 生成哈夫曼编码
    huffman_generate(*root);
    auto mapping = huffman_mapping(*root);

    // 压缩数据
    std::string bit_string;
    for (const auto& b : bpe.bin_list) {
        if (mapping.find(b) != mapping.end()) {
            bit_string += mapping[b];
        }
    }

    // 转换为字节序列
    std::vector<uint8_t> binary_data;
    binary_data.reserve((bit_string.length() + 7) / 8);
    
    uint8_t current_byte = 0;
    int bit_count = 0;
    
    for(char bit : bit_string) {
        current_byte = (current_byte << 1) | (bit - '0');
        bit_count++;
        
        if(bit_count == 8) {
            binary_data.push_back(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }
    
    // 处理最后一个不完整字节
    if(bit_count > 0) {
        current_byte <<= (8 - bit_count);
        binary_data.push_back(current_byte);
    }

    // 保存压缩文件
    bpe.content = binary_data;
    bpe.file_generate_with_header(output_file, "HFC");
}

void HuffmanCompressor::parse_file(const std::string& filename) {
    std::vector<unsigned char> buffer = readFile(filename);
    size_t offset = 0;
    size_t buffer_size = buffer.size();

    // 边界检查
    if (buffer_size < 7) { // 3(signature) + 4(vocab_size)
        throw std::runtime_error("Invalid file format: file too small");
    }

    // 读取文件签名
    std::string signature(buffer.begin(), buffer.begin() + 3);
    offset += 3;
    if (signature != "HFC") {
        throw std::runtime_error("Invalid file signature");
    }

    // 读取词典大小
    uint32_t vocab_size = readData<uint32_t>(buffer, offset);
    std::cerr << "Vocab size: " << vocab_size << std::endl;

    // 读取词典
    vocab.clear();
    for (uint32_t i = 0; i < vocab_size; ++i) {
        // 边界检查
        if (offset + sizeof(uint16_t) > buffer_size) {
            throw std::runtime_error("Unexpected end of file while reading sequence length");
        }

        // 读取序列长度
        uint16_t seq_size = readData<uint16_t>(buffer, offset);
        std::cerr << "Seq size: " << seq_size << std::endl;

        // 边界检查
        if (offset + seq_size + sizeof(uint16_t) > buffer_size) {
            throw std::runtime_error("Unexpected end of file while reading sequence data");
        }

        // 读取序列内容
        std::vector<uint8_t> seq;
        seq.reserve(seq_size);
        for (uint16_t j = 0; j < seq_size; ++j) {
            seq.push_back(buffer[offset++]);
        }

        // 读取频率
        uint16_t freq = readData<uint16_t>(buffer, offset);
        std::cerr << "Freq: " << freq << std::endl;

        vocab[seq] = freq;
    }

    // 读取压缩内容
    if (offset + sizeof(uint32_t) + sizeof(uint8_t) > buffer_size) {
        throw std::runtime_error("Unexpected end of file while reading content size");
    }

    // 读取内容大小和有效位数
    uint32_t content_size = readData<uint32_t>(buffer, offset);
    uint8_t valid_bits = readData<uint8_t>(buffer, offset);

    // 边界检查
    if (offset + content_size > buffer_size) {
        throw std::runtime_error("Unexpected end of file while reading content");
    }

    // 读取内容
    content.clear();
    if (content_size > 0) {
        content.assign(buffer.begin() + offset, 
                      buffer.begin() + offset + content_size);
    }
}

// 修改decompress_file函数中的关键部分
void HuffmanCompressor::decompress_file(const std::string& input_file, const std::string& output_file) {
    try {
        // 解析压缩文件
        HuffmanCompressor decompressor("", 1);
        decompressor.parse_file(input_file);

        // 还原词典和构建哈夫曼树
        std::shared_ptr<Node> root = decompressor.build();
        if (!root) {
            throw std::runtime_error("Failed to build Huffman tree");
        }

        huffman_generate(*root);
        auto mapping = huffman_mapping(*root);

        // 转换压缩数据为位串
        std::string bit_string;
        for (uint8_t byte : decompressor.content) {
            std::bitset<8> bits(byte);
            bit_string += bits.to_string();
        }

        // 解码
        std::vector<uint8_t> decoded_data;
        std::shared_ptr<Node> current = root;
        
        for (char bit : bit_string) {
            if (!current) {
                throw std::runtime_error("Invalid Huffman tree state");
            }

            if (bit == '0') {
                current = current->left;
            } else {
                current = current->right;
            }

            if (current && current->left == nullptr && current->right == nullptr) {
                decoded_data.insert(decoded_data.end(), 
                                  current->seq.begin(), 
                                  current->seq.end());
                current = root;
            }
        }

        // 转换回原始数据
        std::vector<uint8_t> original_data;
        original_data.reserve(decoded_data.size() / 2);

        for (size_t i = 0; i < decoded_data.size() - 1; i += 2) {
            uint8_t byte = (decoded_data[i] << 4) | decoded_data[i + 1];
            original_data.push_back(byte);
        }

        // 写入文件
        std::ofstream out(output_file, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Failed to open output file");
        }

        out.write(reinterpret_cast<const char*>(original_data.data()), 
                 original_data.size());
        out.close();

    } catch (const std::exception& e) {
        std::cerr << "Error in decompress_file: " << e.what() << std::endl;
        throw;
    }
}