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


int main(){
    auto ex = encoding("IT之家 12 月 19 日消息，据日经周三报道，本田与日产正就合并问题展开谈判，以应对富士康（鸿海精密工业）可能发起的收购威胁。\
\
富士康的主营业务是合同制造，主要为苹果生产 iPhone，但它早在 2019 年就宣布进入电动汽车行业。富士康此番试图收购日产股份的举动已经在业内引起关注。据悉，这一动作已经持续了几个月。\
\
日产得知富士康的行动后，进行了紧急的幕后磋商，寻求应对之策，今年 8 月与日产建立战略合作伙伴关系的本田也对富士康的举动表示担忧。一位本田高管严厉警告日产：“如果日产与富士康合作，我们的合作关系将被取消。”本田同样担心，其增长战略的核心 —— 这一伙伴关系，会被富士康的举动破坏。如果鸿海在遭到拒绝后转为敌意收购，本田还将以“白衣骑士”身份保护日产汽车。\
\
据IT之家此前报道，日产、本田以及可能涉及的三菱汽车正在寻求一种结构，以支持未来在自动驾驶和电动汽车电池方面的重大投资。\
\
这三家公司若联合，将成为全球第三大汽车集团，年销量超过 800 万辆"
    );
    BPE a(ex, 8);
    a.train(3);
    a.build();
    for(const auto& i : a.vocab ){
        std::cout << i.first << ":" << i.second << std::endl;
    }


}