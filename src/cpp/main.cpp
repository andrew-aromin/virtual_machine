#include <iostream>
#include "LC3.hpp"
#include "Terminal.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: lc3_cpp <image-file1.obj> [image-file2.obj ...]\n";
        return 1;
    }

    // RAII Terminal guard handles raw mode setup, teardown, and signals automatically
    lc3::Terminal terminal;

    // Configure LC3 with Terminal I/O functions
    lc3::IOHandler io_handler{
        lc3::Terminal::check_key,
        lc3::Terminal::read_char,
        lc3::Terminal::write_char
    };

    lc3::LC3 vm(io_handler);

    // Load each image file passed on command line
    for (int i = 1; i < argc; ++i) {
        if (!vm.load_program(argv[i])) {
            std::cerr << "Failed to load image: " << argv[i] << std::endl;
            return 1;
        }
    }

    vm.run();

    return 0;
}
