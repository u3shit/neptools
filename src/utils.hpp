#ifndef UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#define UUID_9AE0723F_DD0B_434B_8880_D7981FAF1F20
#pragma once

#include "fs.hpp"
#include <iosfwd>
#include <boost/assert.hpp>

using Byte = unsigned char;

// if you want to change it to 64-bit: change FileMemSize to size_t,
// #define _FILE_OFFSET_BITS 64
// to the beginning of source.cpp
using FilePosition = std::uint32_t;
using FileMemSize = std::uint32_t; // min(FilePos, size_t)

template <typename T, typename P>
void EraseIf(T& cnt, P fun)
{
    for (auto it = cnt.begin(); it != cnt.end(); )
    {
        if (fun(*it))
            it = cnt.erase(it);
        else
            ++it;
    }
}

// http://ficksworkshop.com/blog/14-coding/86-how-to-static-cast-std-unique-ptr
template<typename D, typename B>
std::unique_ptr<D> static_cast_ptr(std::unique_ptr<B>&& base)
{
    return std::unique_ptr<D>(static_cast<D*>(base.release()));
}

template <typename T, typename U>
T asserted_cast(U* ptr)
{
    BOOST_ASSERT(dynamic_cast<T>(ptr));
    return static_cast<T>(ptr);
}

std::ofstream OpenOut(const fs::path& pth);
std::ifstream OpenIn(const fs::path& pth);

void DumpBytes(std::ostream& os, const Byte* data, size_t len);
inline void DumpBytes(std::ostream& os, const char* data, size_t len)
{ DumpBytes(os, reinterpret_cast<const Byte*>(data), len); }
inline void DumpBytes(std::ostream& os, const std::string& str)
{ DumpBytes(os, str.data(), str.size()); }

#define STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

#endif
