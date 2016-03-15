#ifndef UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#define UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#pragma once

#include "utils.hpp"
#include "except.hpp"

namespace Neptools
{

struct Pattern
{
    const Byte* pattern;
    const Byte* mask;
    size_t size;

    const Byte* MaybeFind(const Byte* data, size_t size) const noexcept;
    const Byte* MaybeFind(const std::string& str) const noexcept
    { return MaybeFind(reinterpret_cast<const Byte*>(str.data()), str.size()); }

    const Byte* Find(const Byte* data, size_t size) const
    {
        auto ret = MaybeFind(data, size);
        if (!ret) NEPTOOLS_THROW(std::runtime_error{"Couldn't find pattern"});
        return ret;
    }
    const Byte* Find(const std::string& str)
    { return Find(reinterpret_cast<const Byte*>(str.data()), str.size()); }


    // non-const
    Byte* MaybeFind(Byte* data, size_t size) const noexcept
    { return const_cast<Byte*>(MaybeFind(const_cast<const Byte*>(data), size)); }
    Byte* MaybeFind(std::string& str) noexcept
    { return const_cast<Byte*>(MaybeFind(const_cast<const std::string&>(str)));}

    Byte* Find(Byte* data, size_t size) const
    { return const_cast<Byte*>(Find(const_cast<const Byte*>(data), size)); }
    Byte* Find(std::string& str)
    { return const_cast<Byte*>(Find(const_cast<const std::string&>(str)));}
};

}
#endif
