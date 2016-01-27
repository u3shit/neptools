#include "item.hpp"
#include "stcm/file.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " file_name" << std::endl;
        return 1;
    }

    //RawItem item(ReadFile(argv[1]));
    //std::cout << item;
    Stcm::File ctx{argv[1]};
    std::cout << ctx;
}
