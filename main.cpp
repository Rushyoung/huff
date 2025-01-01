// #include "file_huffman.hpp"
//
// int main(){
//     file_huffman::compressFile("z24.png", "3.bin");
//     file_huffman::decompressFile("3.bin", "4.png");
// //    file_huffman::compressFile("1.txt", "2.bin");
// //    file_huffman::decompressFile("2.bin", "3.txt");
// //    file_huffman::compressFile("huff.exe", "huff.bin");
// //    file_huffman::decompressFile("huff.bin", "huff2.exe");
// }
//


#include "bpe.hpp"
#include <iostream>


int main() {
    // std::string input_file = "1.txt";
    // std::string compressed_file = "compressed.bin";
    // std::string decompressed_file = "decompressed.txt";
    //
    // try {
    //     // 压缩文件
    //     HuffmanCompressor::compress_file(input_file, compressed_file, 4, 2);
    //     std::cout << "File compressed successfully." << std::endl;
    //
    //     // 解压缩文件
    //     // HuffmanCompressor::decompress_file(compressed_file, decompressed_file);
    //     // std::cout << "File decompressed successfully." << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }

    std::string input = encoding("9、红黑平衡二叉树及其应用\
【问题描述】\
一棵红黑树本身就是一棵二叉排序树。红黑树中的结点颜色是黑色或红色。从红黑树的根结点到叶子结点的路径上的黑色结点数目相同，最短的路径就是所有结点都是黑色。根据红黑树的性质，红黑树在平衡类二叉排序树中有许多应用。\
【设计要求】\
设计红黑平衡二叉树实现动态查找表及其应用。\
（1）采用STL红黑平衡二叉树数据结构。\
（2）应用基本运算，实现红黑树的简单应用。\
10、动态选择求第k小元素问题\
【问题描述】\
集合中的元素是动态构成的。要求对集合中的n个元素求解第k小的元素。\
【设计要求】\
设计应用红黑平衡二叉树实现动态查找第k小的元素。\
（1）实现动态查找表的三种基本功能：查找、插入、删除。\
（2）应用基本运算，设计算法求解。\
11、基于紧缩图的邻接表的拓扑排序问题\
【问题描述】\
紧缩邻接表将图的每个顶点的邻接表紧凑的存储在两个向量list和h中。其中向量list依次存储顶点0，1，…，n-1的邻接顶点。向量单元h[i]存储顶点i的邻接表在向量list中的起始位置。\
【设计要求】\
设计基于紧缩图的邻接表的拓扑排序程序。\
（1）采用STL的图、紧缩邻接表、栈等数据结构。\
（2）应用基本运算，设计算法求解。\
");
    std::cout << input << std::endl;
    HuffmanCompressor a(input, 2);
    a.train(3);
    a.build();


    return 0;
}