#include "progargs.hpp"
#include <iostream>


int main() {
    // Define test cases as arrays of arguments
    const char* args1[] = { "imtool", "input.ppm", "output.ppm", "info" };
    const char* args2[] = { "imtool", "input.ppm", "output.ppm", "maxlevel", "128" };
    const char* args3[] = { "imtool", "input.ppm", "output.ppm", "resize", "800", "600" };
    const char* args4[] = { "imtool", "input.ppm", "output.ppm", "cutfreq", "10" };
    const char* args5[] = { "imtool", "input.ppm", "output.ppm", "compress" };
    const char* invalidArgs[] = { "imtool", "input.ppm", "output.ppm", "unknown" };


    // Run each test case
    try {
        auto progArgs1 = ProgArgs::parse_arguments(4, const_cast<char**>(args1));
        std::cout << "Test info passed: " << progArgs1.getOperation() << std::endl;


        auto progArgs2 = ProgArgs::parse_arguments(5, const_cast<char**>(args2));
        std::cout << "Test maxlevel passed: " << progArgs2.getOperation() << std::endl;


        auto progArgs3 = ProgArgs::parse_arguments(6, const_cast<char**>(args3));
        std::cout << "Test resize passed: " << progArgs3.getOperation() << std::endl;


        auto progArgs4 = ProgArgs::parse_arguments(5, const_cast<char**>(args4));
        std::cout << "Test cutfreq passed: " << progArgs4.getOperation() << std::endl;


        auto progArgs5 = ProgArgs::parse_arguments(4, const_cast<char**>(args5));
        std::cout << "Test compress passed: " << progArgs5.getOperation() << std::endl;


        // Invalid operation test
        auto invalidProgArgs = ProgArgs::parse_arguments(4, const_cast<char**>(invalidArgs));
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }


    return 0;
}
