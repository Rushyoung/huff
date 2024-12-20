#include <string>

#include <iostream>

#include "args.hpp"
#include "huff.hpp"
#include "jsrt.hpp"

args::arg parser_args(int argc, char *argv[]) {
    args::settings parser(
        args::option("-c", "--compress"  , "Compress a file")
            .as(args::type::string),
        args::option("-d", "--decompress", "Decompress a file")
            .as(args::type::string),
        args::option("-o", "--output"    , "Output file")   
            .as(args::type::string)
            .default_val("output.hff"),
        args::option("--debug", "enable debug mode for more information")
            .as(args::type::boolean)
            .default_val("false")
    );

    return parser.parse(argc, argv);
}

int main(int argc, char *argv[]) {
    JS_Run();
    args::arg result;
    try{
        result = parser_args(argc, argv);
    } catch(...) {
        std::cerr << "Error parsing arguments" << std::endl;
        return 1;
    }

    if(result.has("compress")) {
        std::string file   = result["compress"].as<args::string>();
        std::string output = result["output"].as<args::string>();
        bool debug = result["debug"].as<args::boolean>();
        return huff::compress(file, output, debug);
    }
    
    if(result.has("decompress")) {
        std::string file   = result["decompress"].as<args::string>();
        bool debug = result["debug"].as<args::boolean>();
        return huff::decompress(file, debug);
    }
}

