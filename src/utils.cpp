#include "utils.hpp"
#include <fstream>

// workaround incompatibilities between msvc filesystem, mingw ofstream (no wide
// char open) and linux...
#ifndef BOOST_FILESYSTEM_C_STR
#define BOOST_FILESYSTEM_C_STR c_str()
#endif

std::ofstream OpenOut(const fs::path& pth)
{
    std::ofstream os;
    os.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    os.open(pth.BOOST_FILESYSTEM_C_STR, std::ios_base::out | std::ios_base::binary);
    return os;
}

std::ifstream OpenIn(const fs::path& pth)
{
    std::ifstream is;
    is.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    is.open(pth.BOOST_FILESYSTEM_C_STR, std::ios_base::in | std::ios_base::binary);
    return is;
}
