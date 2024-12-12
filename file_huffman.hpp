#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <bitset>

using namespace std;

struct HuffmanNode {
    char ch;
    int freq;
    shared_ptr<HuffmanNode> left, right;

    HuffmanNode(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
    HuffmanNode(char ch, int freq, shared_ptr<HuffmanNode> l, shared_ptr<HuffmanNode> r) : ch(ch), freq(freq), left(l), right(r) {}
};

// 比较器，用于优先队列
struct Compare {
    bool operator()(shared_ptr<HuffmanNode> l, shared_ptr<HuffmanNode> r) {
        return l->freq > r->freq;
    }
};

class file_huffman {
private:
    static void generateCodes(shared_ptr<HuffmanNode> root, const string& str, unordered_map<char, string> &huffmanCode) {
        if (!root) return;

        if (!root->left && !root->right) {
            huffmanCode[root->ch] = str;
        }

        generateCodes(root->left, str + "0", huffmanCode);
        generateCodes(root->right, str + "1", huffmanCode);
    }

    static shared_ptr<HuffmanNode> buildHuffmanTree(const unordered_map<char, int> &freq) {
        priority_queue<shared_ptr<HuffmanNode>, vector<shared_ptr<HuffmanNode>>, Compare> pq;

        for (auto pair : freq) {
            pq.push(make_shared<HuffmanNode>(pair.first, pair.second));
        }

        while (pq.size() != 1) {
            auto left = pq.top(); pq.pop();
            auto right = pq.top(); pq.pop();

            int sum = left->freq + right->freq;
            pq.push(make_shared<HuffmanNode>('\0', sum, left, right));
        }

        return pq.top();
    }

    static void saveTree(shared_ptr<HuffmanNode> root, ofstream &out) {
        if (!root) {
            out.put('#');
            return;
        }
        out.put(root->ch);
        saveTree(root->left, out);
        saveTree(root->right, out);
    }

    static shared_ptr<HuffmanNode> loadTree(ifstream &in) {
        char ch;
        in.get(ch);
        if (ch == '#') return nullptr;

        auto node = make_shared<HuffmanNode>(ch, 0);
        node->left = loadTree(in);
        node->right = loadTree(in);
        return node;
    }

public:
    static void compressFile(const string &inputFile, const string &outputFile) {
        ifstream in(inputFile, ios::binary);
        if (!in.is_open()) {
            cerr << "Error opening input file" << endl;
            return;
        }

        ofstream out(outputFile, ios::binary);
        if (!out.is_open()) {
            cerr << "Error opening output file" << endl;
            return;
        }

        unordered_map<char, int> freq;
        char ch;
        while (in.get(ch)) {
            freq[ch]++;
        }

        auto root = buildHuffmanTree(freq);

        unordered_map<char, string> huffmanCode;
        generateCodes(root, "", huffmanCode);

        in.clear();
        in.seekg(0, ios::beg);

        string encodedStr;
        while (in.get(ch)) {
            encodedStr += huffmanCode[ch];
        }

        saveTree(root, out);
        out.put('#'); // Tree and data separator

        bitset<8> bits;
        int bitIndex = 0;
        for (char bit : encodedStr) {
            bits[bitIndex++] = bit - '0';
            if (bitIndex == 8) {
                out.put(static_cast<char>(bits.to_ulong()));
                bits.reset();
                bitIndex = 0;
            }
        }
        if (bitIndex > 0) {
            out.put(static_cast<char>(bits.to_ulong()));
        }

        in.close();
        out.close();
    }

    static void decompressFile(const string& inputFile, const string& outputFile) {
        ifstream in(inputFile, ios::binary);
        if (!in.is_open()) {
            cerr << "Error opening input file" << endl;
            return;
        }

        ofstream out(outputFile, ios::binary);
        if (!out.is_open()) {
            cerr << "Error opening output file" << endl;
            return;
        }

        auto root = loadTree(in);
        in.get(); // Skip the tree and data separator

        string encodedStr;
        char byte;
        while (in.get(byte)) {
            bitset<8> bits(byte);
            for (int i = 0; i < 8; ++i) {
                encodedStr += bits[i] ? '1' : '0';
            }
        }

        auto current = root;
        for (char bit : encodedStr) {
            if (bit == '0') {
                current = current->left;
            } else {
                current = current->right;
            }

            if (!current->left && !current->right) {
                out.put(current->ch);
                current = root;
            }
        }

        in.close();
        out.close();
    }
};