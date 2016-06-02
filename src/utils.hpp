#ifndef UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#define UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#pragma once

#include <iosfwd>
#include <boost/filesystem/path.hpp>
#include "except.hpp"

namespace Neptools
{

using Byte = unsigned char;

// if you want to change it to 64-bit: change FileMemSize to size_t,
// #define _FILE_OFFSET_BITS 64
// to the beginning of source.cpp, and fix braindead winapi
using FilePosition = uint32_t;
using FileMemSize = uint32_t; // min(FilePos, size_t)

template <typename T, typename U>
T asserted_cast(U* ptr)
{
    NEPTOOLS_ASSERT_MSG(dynamic_cast<T>(ptr), "U is not T");
    return static_cast<T>(ptr);
}

template <typename T>
constexpr size_t EmptySizeof = std::is_empty<T>::value ? 0 : sizeof(T);

std::ofstream OpenOut(const boost::filesystem::path& pth);
std::ifstream OpenIn(const boost::filesystem::path& pth);

void DumpBytes(std::ostream& os, const Byte* data, size_t len);
inline void DumpBytes(std::ostream& os, const char* data, size_t len)
{ DumpBytes(os, reinterpret_cast<const Byte*>(data), len); }
inline void DumpBytes(std::ostream& os, const std::string& str)
{ DumpBytes(os, str.data(), str.size()); }

#define NEPTOOLS_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

}
#endif
