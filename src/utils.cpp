#include "utils.hpp"

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

  static void DumpByte(std::ostream& os, char c)
  {
    if (c == '"')
      os << "\\\"";
    else if (c == '\\')
      os << "\\\\";
    else if (c == '\n')
      os << "\\n";
    else if (c == '\r')
      os << "\\r";
    else if (c >= ' ' && c <= '~')
      os << c;
    else
      os << "\\x" << std::setw(2) << unsigned(static_cast<unsigned char>(c));
  }

  void DumpBytes(std::ostream& os, Libshit::StringView data)
  {
    auto flags = os.flags();
    os << std::hex << std::setfill('0') << '"';
    for (size_t i = 0; i < data.length(); ++i) DumpByte(os, data[i]);
    os << '"';
    os.flags(flags);
  }

  void DumpBytes(std::ostream& os, Source data)
  {
    auto flags = os.flags();
    os << std::hex << std::setfill('0') << '"';

    for (FilePosition offs = 0, size = data.GetSize(); offs < size; )
    {
      auto chunk = data.GetChunk(offs);
      for (char c : chunk) DumpByte(os, c);
      offs += chunk.length();
    }
    os << '"';
    os.flags(flags);
  }

}
