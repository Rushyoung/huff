#include "bpe.hpp"
#include <iomanip>
#include <sstream>
#include <queue>

std::vector<uint8_t> HuffmanCompressor::hex_string_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        uint8_t high = (hex[i] >= 'A') ? (hex[i] - 'A' + 10) : (hex[i] - '0');
        uint8_t low = (hex[i+1] >= 'A') ? (hex[i+1] - 'A' + 10) : (hex[i+1] - '0');
        bytes.push_back((high << 4) | low);
    }
    return bytes;
}

std::vector<uint8_t> HuffmanCompressor::hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.length() / 2);
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        uint8_t high = (hex[i] >= 'A') ? (hex[i] - 'A' + 10) : (hex[i] - '0');
        uint8_t low = (hex[i+1] >= 'A') ? (hex[i+1] - 'A' + 10) : (hex[i+1] - '0');
        bytes.push_back((high << 4) | low);
    }
    return bytes;
}

std::string HuffmanCompressor::bytes_to_hex_string(const std::vector<uint8_t>& bytes) {
    static const char hex_chars[] = "0123456789ABCDEF";
    std::string hex;
    hex.reserve(bytes.size() * 2);
    
    for (uint8_t byte : bytes) {
        hex.push_back(hex_chars[byte >> 4]);
        hex.push_back(hex_chars[byte & 0x0F]);
    }
    return hex;
}

std::string HuffmanCompressor::hex_to_bits_aligned(const std::string& hex, uint8_t valid_bits) {
    std::string bits;
    bits.reserve(hex.length() * 4);
    
    // 处理完整字节
    for (size_t i = 0; i < hex.length() - 2; i += 2) {
        uint8_t byte = (hex[i] >= 'A' ? hex[i] - 'A' + 10 : hex[i] - '0') << 4 |
                      (hex[i+1] >= 'A' ? hex[i+1] - 'A' + 10 : hex[i+1] - '0');
        std::bitset<8> bitset(byte);
        bits += bitset.to_string();
    }
    
    // 处理最后一个字节
    if (!hex.empty()) {
        uint8_t last_byte = (hex[hex.length()-2] >= 'A' ? hex[hex.length()-2] - 'A' + 10 : hex[hex.length()-2] - '0') << 4 |
                           (hex[hex.length()-1] >= 'A' ? hex[hex.length()-1] - 'A' + 10 : hex[hex.length()-1] - '0');
        std::bitset<8> last_bits(last_byte);
        bits += last_bits.to_string().substr(0, valid_bits);
    }
    
    return bits;
}

std::unordered_map<std::string, std::string> HuffmanCompressor::mapping;
// std::string encoding(const std::string& str) {
//     std::string result;
//     for (unsigned char c : str) {
//         std::stringstream ss;
//         ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
//         result += ss.str();
//     }
//     return result;
// }

std::string encoding(const std::string& str) {
    static const char hex_chars[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(str.length() * 2);
    for (unsigned char c : str) {
        result.push_back(hex_chars[c >> 4]);
        result.push_back(hex_chars[c & 0x0F]);
    }
    return result;
}

std::string decoding(const std::string& str) {
    if (str.empty() || str.length() % 2 != 0) {
        throw std::runtime_error("Invalid hex string length");
    }
    
    std::string result;
    result.reserve(str.length() / 2);
    
    for (size_t i = 0; i < str.length(); i += 2) {
        uint8_t high = (str[i] >= 'A') ? (str[i] - 'A' + 10) : (str[i] - '0');
        uint8_t low = (str[i+1] >= 'A') ? (str[i+1] - 'A' + 10) : (str[i+1] - '0');
        result.push_back((high << 4) | low);
    }
    return result;
}


std::string HuffmanCompressor::bits_to_hex(const std::string& bits) {
    std::string hex;
    hex.reserve((bits.length() + 7) / 8 * 2);
    
    for (size_t i = 0; i < bits.length(); i += 8) {
        std::string byte_str = bits.substr(i, std::min(size_t(8), bits.length() - i));
        while (byte_str.length() < 8) {
            byte_str += '0';  // 右补0
        }
        uint8_t byte = std::bitset<8>(byte_str).to_ulong();
        char hex_byte[3];
        snprintf(hex_byte, sizeof(hex_byte), "%02X", byte);
        hex += hex_byte;
    }
    return hex;
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
    if (!out) throw std::runtime_error("Failed to open output file");

    // 1. 写入文件头和词典大小
    out.write(head.c_str(), 3);
    uint32_t vocab_size = vocab.size();
    out.write(reinterpret_cast<const char*>(&vocab_size), sizeof(vocab_size));

    // 2. 写入词典项
    for(const auto& [seq, freq] : vocab) {
        uint16_t seq_size = seq.length() / 2;  // 实际字节数
        out.write(reinterpret_cast<const char*>(&seq_size), sizeof(seq_size));
        
        // 直接转换并写入二进制
        auto bytes = hex_to_bytes(seq);
        out.write(reinterpret_cast<const char*>(bytes.data()), seq_size);
        
        uint16_t freq_val = static_cast<uint16_t>(freq);
        out.write(reinterpret_cast<const char*>(&freq_val), sizeof(freq_val));
    }

    // 3. 写入压缩内容
    auto content_bytes = hex_to_bytes(compressed_hex);
    uint32_t content_size = content_bytes.size();
    out.write(reinterpret_cast<const char*>(&content_size), sizeof(content_size));
    out.write(reinterpret_cast<const char*>(&valid_bits), sizeof(valid_bits));
    
    if (!content_bytes.empty()) {
        out.write(reinterpret_cast<const char*>(content_bytes.data()), content_size);
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
HuffmanCompressor::HuffmanCompressor(const std::string& input, int batch) {
    bin = encoding(input);
    for (size_t i = 0; i < bin.length(); i += batch) {
        bin_list.push_back(bin.substr(i, batch));
    }
}
void HuffmanCompressor::printTree(const std::shared_ptr<Node>& root, int depth) {
    if (!root) return;

    std::cout << std::string(depth * 4, ' ') 
              << "Freq: " << root->freq
              << ", Seq: " << root->seq
              << ", Code: " << root->code << std::endl;

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

    // 预分配空间以减少重新分配
    size_t estimated_pairs = bin_list.size() - 1;
    pairs.reserve(estimated_pairs);
    pair_pos.reserve(estimated_pairs);

    // 使用滑动窗口计算对
    for (size_t i = 1; i < bin_list.size(); ++i) {
        std::pair<std::string, std::string> pair{bin_list[i - 1], bin_list[i]};
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
    if (pairs.empty()) return;

    // 1. 找到最高频率的相邻对
    auto best_pair = std::max_element(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->first;

    // 2. 检查是否在黑名单中
    while (blacklist.find(best_pair) != blacklist.end()) {
        pairs.erase(best_pair);
        if (pairs.empty()) return;
        best_pair = std::max_element(pairs.begin(), pairs.end(),
            [](const auto& a, const auto& b) {
                return a.second < b.second;
            })->first;
    }

    if (pairs[best_pair] <= min_freq) return;

    // 3. 执行合并操作
    std::string best_bin = best_pair.first + best_pair.second;
    auto old_list = bin_list;  // 保存原始列表以便回滚
    size_t length = bin_list.size();
    size_t offset = 0;

    // 4. 更新bin_list
    for (int pos : pair_pos[best_pair]) {
        size_t idx = pos - offset;
        if (idx >= length - 1) continue;
        
        if (bin_list[idx] == best_pair.first && 
            bin_list[idx + 1] == best_pair.second) {
            bin_list[idx] = best_bin;
            bin_list.erase(bin_list.begin() + idx + 1);
            offset++;
        }
    }

    // 5. 更新词频
    int freq = pairs[best_pair];
    token[best_bin] = freq;
    token[best_pair.first] -= freq;
    token[best_pair.second] -= freq;

    // 6. 检查合并是否有效
    auto check_freq = [min_freq](int freq) {
        return freq > 0 && freq > min_freq;
    };

    if (!check_freq(token[best_pair.first]) || 
        !check_freq(token[best_pair.second])) {
        // 合并无效，回滚
        bin_list = std::move(old_list);
        token[best_bin] = 0;
        token[best_pair.first] += freq;
        token[best_pair.second] += freq;
        blacklist.insert(best_pair);
    }
}

void HuffmanCompressor::train(int min_freq, int size) {
    if (size == -1) {
        size = std::min(256, static_cast<int>(bin.length() / 64));
    }

    // 1. 初始化词典
    deal_vocab();
    deal_token();

    // 2. BPE训练循环
    for (int i = 0; i < size; i++) {
        // 计算相邻对
        deal_pairs();
        if (pairs.empty()) break;

        // 合并最频繁的对
        merge_vocab(min_freq);
        
        // 调试输出
        std::cerr << "Iteration " << i + 1 
                  << ", vocab size: " << vocab.size() 
                  << ", pairs: " << pairs.size() << std::endl;
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
    if (vocab.empty()) {
        return nullptr;
    }

    // 使用最小堆优化排序
    auto comp = [](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
        return a->freq * a->seq.length() > b->freq * b->seq.length();
    };
    std::priority_queue<std::shared_ptr<Node>, 
                       std::vector<std::shared_ptr<Node>>, 
                       decltype(comp)> nodes(comp);

    // 初始化节点

    for (const auto& [seq, freq] : vocab) {
        nodes.push(std::make_shared<Node>(freq, seq));
    }

    // 构建哈夫曼树
    while (nodes.size() > 1) {
        auto left = nodes.top(); nodes.pop();
        auto right = nodes.top(); nodes.pop();

        auto parent = std::make_shared<Node>(left->freq + right->freq, "");
        parent->left = left;
        parent->right = right;
        nodes.push(parent);
    }

    return nodes.empty() ? nullptr : nodes.top();
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

std::unordered_map<std::string, std::string> HuffmanCompressor::huffman_mapping(Node& n) {
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
    std::cerr << "开始压缩文件: " << input_file << std::endl;
    
    // 读取并初始化
    std::ifstream in(input_file, std::ios::binary);
    if (!in) throw std::runtime_error("Error opening input file");
    std::string input_str((std::istreambuf_iterator<char>(in)), 
                         std::istreambuf_iterator<char>());
    in.close();
    
    std::cerr << "原始文件大小: " << input_str.length() << " 字节" << std::endl;

    // BPE处理
    HuffmanCompressor bpe(input_str, batch);
    std::cerr << "初始化BPE, batch大小: " << batch << std::endl;
    
    bpe.train(min_freq);
    std::cerr << "BPE训练完成, 最小频率: " << min_freq << std::endl;
    std::cerr << "词典大小: " << bpe.vocab.size() << std::endl;

    auto root = bpe.build();
    if (!root) throw std::runtime_error("Failed to build Huffman tree");
    std::cerr << "哈夫曼树构建完成" << std::endl;

    // 生成映射
    huffman_generate(*root);
    auto mapping = huffman_mapping(*root);
    std::cerr << "编码映射表大小: " << mapping.size() << std::endl;

    // 压缩数据
    std::string bit_string;
    for (const auto& b : bpe.bin_list) {
        if (auto it = mapping.find(b); it != mapping.end()) {
            bit_string += it->second;
        }
    }
    std::cerr << "生成位串长度: " << bit_string.length() << " bits" << std::endl;

    // 记录有效位数
    bpe.valid_bits = bit_string.length() % 8;
    if (bpe.valid_bits == 0) bpe.valid_bits = 8;
    std::cerr << "最后一个字节有效位数: " << (int)bpe.valid_bits << std::endl;

    // 转换为十六进制
    bpe.compressed_hex = bits_to_hex(bit_string);
    std::cerr << "压缩后大小: " << bpe.compressed_hex.length() / 2 << " 字节" << std::endl;
    std::cerr << "压缩率: " << (float)(bpe.compressed_hex.length() / 2) / input_str.length() * 100 << "%" << std::endl;
    
    // 写入文件
    bpe.file_generate_with_header(output_file, "HFC");
    std::cerr << "压缩完成，已保存到: " << output_file << std::endl;
}


void HuffmanCompressor::parse_file(const std::string& filename) {
    std::vector<unsigned char> buffer = readFile(filename);
    size_t offset = 0;

    if (buffer.size() < 8) throw std::runtime_error("Invalid file format");

    // 1. 检查文件头
    std::string signature(buffer.begin(), buffer.begin() + 3);
    offset += 3;
    if (signature != "HFC") throw std::runtime_error("Invalid file signature");

    // 2. 读取词典大小
    uint32_t vocab_size = readData<uint32_t>(buffer, offset);
    vocab.clear();

    // 3. 读取词典项
    for (uint32_t i = 0; i < vocab_size; ++i) {
        // 读取序列长度
        uint16_t seq_size = readData<uint16_t>(buffer, offset);
        
        // 转换二进制序列为十六进制字符串
        std::string seq;
        seq.reserve(seq_size * 2);  // 预分配空间
        for (uint16_t j = 0; j < seq_size; ++j) {
            static const char hex_chars[] = "0123456789ABCDEF";
            uint8_t byte = buffer[offset++];
            seq.push_back(hex_chars[byte >> 4]);
            seq.push_back(hex_chars[byte & 0x0F]);
        }

        // 读取频率
        uint16_t freq = readData<uint16_t>(buffer, offset);
        vocab[seq] = freq;
    }

    // 4. 读取压缩内容信息
    uint32_t content_size = readData<uint32_t>(buffer, offset);
    valid_bits = readData<uint8_t>(buffer, offset);

    // 5. 读取压缩内容并转换为十六进制字符串
    compressed_hex.clear();
    compressed_hex.reserve(content_size * 2);
    static const char hex_chars[] = "0123456789ABCDEF";
    
    for (uint32_t i = 0; i < content_size; ++i) {
        uint8_t byte = buffer[offset + i];
        compressed_hex.push_back(hex_chars[byte >> 4]);
        compressed_hex.push_back(hex_chars[byte & 0x0F]);
    }
}

// 修改decompress_file函数中的关键部分
void HuffmanCompressor::decompress_file(const std::string& input_file, const std::string& output_file) {
    try {
        std::cerr << "开始解压文件: " << input_file << std::endl;

        HuffmanCompressor decompressor("", 1);
        decompressor.parse_file(input_file);
        std::cerr << "文件解析完成" << std::endl;
        std::cerr << "词典大小: " << decompressor.vocab.size() << std::endl;

        // 构建哈夫曼树
        auto root = decompressor.build();
        if (!root) throw std::runtime_error("Failed to build Huffman tree");
        std::cerr << "哈夫曼树重建完成" << std::endl;

        // 生成编码映射
        huffman_generate(*root);
        auto mapping = huffman_mapping(*root);
        std::cerr << "编码映射表重建完成，大小: " << mapping.size() << std::endl;

        // 构建反向映射
        std::unordered_map<std::string, std::string> reverse_mapping;
        for (const auto& [seq, code] : mapping) {
            reverse_mapping[code] = seq;
        }

        // 解码压缩内容
        std::cerr << "压缩内容大小: " << decompressor.compressed_hex.length() / 2 << " 字节" << std::endl;
        std::cerr << "最后一个字节有效位数: " << (int)decompressor.valid_bits << std::endl;

        std::string bit_string = hex_to_bits_aligned(decompressor.compressed_hex, 
                                                   decompressor.valid_bits);
        std::cerr << "解压后位串长度: " << bit_string.length() << " bits" << std::endl;

        // 解码
        std::string decoded_hex;
        decoded_hex.reserve(bit_string.length());
        std::string current_code;
        current_code.reserve(32);
        
        size_t decoded_symbols = 0;
        for (char bit : bit_string) {
            current_code += bit;
            if (auto it = reverse_mapping.find(current_code); it != reverse_mapping.end()) {
                decoded_hex += it->second;
                current_code.clear();
                decoded_symbols++;
            }
        }
        std::cerr << "解码的符号数量: " << decoded_symbols << std::endl;

        // 转换并写入文件
        std::string original_text = decoding(decoded_hex);
        std::cerr << "解压后文件大小: " << original_text.length() << " 字节" << std::endl;

        std::ofstream out(output_file, std::ios::binary);
        if (!out) throw std::runtime_error("Failed to open output file");
        out.write(original_text.c_str(), original_text.length());
        out.close();

        std::cerr << "解压完成，已保存到: " << output_file << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "解压错误: " << e.what() << std::endl;
        throw;
    }
}