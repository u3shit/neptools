#include "utils.hpp"
#include <boost/filesystem/fstream.hpp>

// BOOST_FILESYSTEM_C_STR: workaround the fact that boost::filesystem developers
// still live in 1998.
std::ofstream OpenOut(const boost::filesystem::path& pth)
{
    std::ofstream os;
    os.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    os.open(pth.BOOST_FILESYSTEM_C_STR, std::ios_base::out | std::ios_base::binary);
    return os;
}

std::ifstream OpenIn(const boost::filesystem::path& pth)
{
    std::ifstream is;
    is.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    is.open(pth.BOOST_FILESYSTEM_C_STR, std::ios_base::in | std::ios_base::binary);
    return is;
}
