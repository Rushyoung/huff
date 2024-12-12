//
// Created by 小小喵姬 on 24-12-12.
//

#ifndef HUFF_FILE_HUFFMAN_HPP
#define HUFF_FILE_HUFFMAN_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>

using namespace std;

struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode *left, *right;

    HuffmanNode(char ch, int freq) : ch(ch), freq(freq), left(nullptr), right(nullptr) {}
    HuffmanNode(char ch, int freq, HuffmanNode* l, HuffmanNode* r) : left(l), right(r), ch(ch), freq(freq){}
};

// 比较器，用于优先队列
struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

class file_huffman {
private:
    static void generateCodes(HuffmanNode* root, const string& str, unordered_map<char, string> &huffmanCode) {
        if (!root) return;

        if (!root->left && !root->right) {
            huffmanCode[root->ch] = str;
        }

        generateCodes(root->left, str + "0", huffmanCode);
        generateCodes(root->right, str + "1", huffmanCode);
    }

    static HuffmanNode* buildHuffmanTree(const unordered_map<char, int> &freq) {
        priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;

        for (auto pair : freq) {
            pq.push(new HuffmanNode(pair.first, pair.second));
        }

        while (pq.size() != 1) {
            HuffmanNode *left = pq.top(); pq.pop();
            HuffmanNode *right = pq.top(); pq.pop();

            int sum = left->freq + right->freq;
            pq.push(new HuffmanNode('\0', sum, left, right));
        }

        return pq.top();
    }
public:
    static void compressFile(const string &inputFile, const string &outputFile) {
        ifstream in(inputFile, ios::binary);
        ofstream out(outputFile, ios::binary);

        unordered_map<char, int> freq;
        char ch;
        while (in.get(ch)) {
            freq[ch]++;
        }

        HuffmanNode* root = buildHuffmanTree(freq);

        unordered_map<char, string> huffmanCode;
        generateCodes(root, "", huffmanCode);

        in.clear();
        in.seekg(0, ios::beg);

        string encodedStr;
        while (in.get(ch)) {
            encodedStr += huffmanCode[ch];
        }

        out << encodedStr;

        in.close();
        out.close();
    }
    void decompressFile(const string& inputFile, const string& output);
};


#endif //HUFF_FILE_HUFFMAN_HPP
