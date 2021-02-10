#ifndef UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#define UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#pragma once

#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <cstdlib>
#include <iosfwd>

namespace Neptools
{
  class Source; // fwd

  using Byte = unsigned char;

  using FilePosition = std::uint64_t;
  using FileMemSize = std::size_t;

  static constexpr const std::size_t MEM_CHUNK  = 8*1024; // 8KiB
  static constexpr const std::size_t MMAP_CHUNK = 128*1024; // 128KiB
  static constexpr const std::size_t MMAP_LIMIT = 1*1024*1024; // 1MiB

  std::ofstream OpenOut(const boost::filesystem::path& pth);
  std::ifstream OpenIn(const boost::filesystem::path& pth);

  void DumpBytes(std::ostream& os, Source data);
}
#endif
