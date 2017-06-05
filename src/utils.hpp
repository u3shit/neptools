#ifndef UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#define UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#pragma once

#include <iosfwd>
#include <boost/filesystem/path.hpp>
#include "except.hpp"
#include "nonowning_string.hpp"

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
    NEPTOOLS_ASSERT_MSG(
        dynamic_cast<T>(ptr) == static_cast<T>(ptr), "U is not T");
    return static_cast<T>(ptr);
}

template <typename T, typename U>
T asserted_cast(U& ref)
{
#ifndef NDEBUG // gcc shut up about unused typedef
    using raw_t = std::remove_reference_t<T>;
    NEPTOOLS_ASSERT_MSG(
        dynamic_cast<raw_t*>(&ref) == static_cast<raw_t*>(&ref), "U is not T");
#endif
    return static_cast<T>(ref);
}

template <typename T>
struct AddConst { using Type = const T; };
template <typename T>
struct AddConst<T*> { using Type = typename AddConst<T>::Type* const; };

template <typename T>
inline T implicit_const_cast(typename AddConst<T>::Type x)
{ return const_cast<T>(x); }

std::ofstream OpenOut(const boost::filesystem::path& pth);
std::ifstream OpenIn(const boost::filesystem::path& pth);

void DumpBytes(std::ostream& os, StringView data);

#define NEPTOOLS_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

}
#endif
