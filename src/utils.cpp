#include "utils.hpp"
#include <libshit/char_utils.hpp>

#include "source.hpp"
#include <fstream>
#include <iomanip>

// workaround incompatibilities between clang+msvc libs, mingw ofstream (no wide
// char open) and linux...
#ifndef BOOST_FILESYSTEM_C_STR
#  define BOOST_FILESYSTEM_C_STR c_str()
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

  void DumpBytes(std::ostream& os, Source data)
  {
    os << '"';

    bool hex = false;
    for (FilePosition offs = 0, size = data.GetSize(); offs < size; )
    {
      auto chunk = data.GetChunk(offs);
      for (char c : chunk)
        hex = Libshit::DumpByte(os, c, hex);
      offs += chunk.length();
    }
    os << '"';
  }

}
