// Reads a hex string from stdin and outputs binary on stdout
#include <iostream>

int main(int argc, char** argv) {
    while (std::cin.peek() != std::istream::traits_type::eof()) {
        std::cin >> std::ws;
        char buff[2];
        std::cin.read(buff, 2);
        std::cout.put((char) std::stoul(buff, NULL, 16));
    }
    return 0;
}
