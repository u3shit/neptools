#include "utils.hpp"
#include <fstream>
#include <iomanip>

// workaround incompatibilities between clang+msvc libs, mingw ofstream (no wide
// char open) and linux...
#ifndef BOOST_FILESYSTEM_C_STR
#define BOOST_FILESYSTEM_C_STR c_str()
#endif

namespace Neptools
{

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

void DumpBytes(std::ostream& os, const Byte* data, size_t len)
{
    auto flags = os.flags();
    os << std::hex << std::setfill('0') << '"';
    for (size_t i = 0; i < len; ++i)
        if (data[i] == '"')
            os << "\\\"";
        else if (data[i] == '\\')
            os << "\\\\";
        else if (data[i] == '\n')
            os << "\\n";
        else if (data[i] == '\r')
            os << "\\r";
        else if (data[i] >= ' ' && data[i] <= '~')
            os << data[i];
        else
            os << "\\x" << std::setw(2) << static_cast<unsigned>(data[i]);
    os << '"';
    os.flags(flags);
}

}
