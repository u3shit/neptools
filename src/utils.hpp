#ifndef UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#define UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#pragma once

#include <boost/filesystem/path.hpp>
#include <fstream>

namespace Neptools
{
  class Source; // fwd

  using Byte = unsigned char;

  // if you want to change it to 64-bit: change FileMemSize to size_t,
  // #define _FILE_OFFSET_BITS 64
  // to the beginning of source.cpp, and fix braindead winapi
  using FilePosition = uint32_t;
  using FileMemSize = uint32_t; // min(FilePos, size_t)

  std::ofstream OpenOut(const boost::filesystem::path& pth);
  std::ifstream OpenIn(const boost::filesystem::path& pth);

  void DumpBytes(std::ostream& os, Source data);
}
#endif
