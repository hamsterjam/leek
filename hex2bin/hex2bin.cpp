// Reads a hex string from stdin and outputs binary on stdout
#include <iostream>
#include <limits>

const std::streamsize maxStreamSize = std::numeric_limits<std::streamsize>::max();

int main(int argc, char** argv) {
    char buff[3];
    buff[2] = 0;
    while (std::cin.peek() != std::istream::traits_type::eof()) {
        std::cin >> std::ws;
        if (std::cin.peek() == '\n') {
            std::cin.get();
            continue;
        }
        if (std::cin.peek() == '#') {
            std::cin.ignore(maxStreamSize, '\n');
            continue;
        }
        std::cin.read(buff, 2);
        std::cout.put((char) std::stoul(buff, NULL, 16));
    }
    return 0;
}
