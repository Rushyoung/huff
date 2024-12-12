#include <string>

#include "args.hpp"
#include "huff.hpp"

int main(int argc, char *argv[]) {
    args::settings parser(
        args::option("-c", "--compress"  , "Compress a file")
            .as(args::type::string)
            .required(),
        args::option("-d", "--decompress", "Decompress a file")
            .as(args::type::string)
            .required(),
        args::option("-o", "--output"    , "Output file")   
            .as(args::type::string)
            .default_val("output.hff")
    )

    args::arg result = parser.parse(argc, argv);
}

