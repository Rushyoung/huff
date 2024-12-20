#pragma once

#include <string>
#include <map>
#include <vector>

using byte = unsigned char;
using dict = std::map<std::string, int>;

namespace huff{

struct bits{
    std::vector<byte> data;
    bits() = default;
};

class file{
    dict vocab;
    bits content;
};

int compress  (std::string, std::string, bool=false);
int decompress(std::string,              bool=false);


}