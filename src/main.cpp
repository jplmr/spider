#include <iostream>
#include <string>

extern int dispvid(int argc, char** argv);

int main(int argc, char** argv) {
    std::cout << "test" << std::endl;
    dispvid(argc, argv);
}
