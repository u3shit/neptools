#ifndef UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#define UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#pragma once

#include "utils.hpp"
#include <libshit/except.hpp>
#include <libshit/nonowning_string.hpp>

namespace Neptools
{

struct Pattern
{
    const Byte* pattern;
    const Byte* mask;
    size_t size;

    const Byte* MaybeFind(StringView data) const noexcept;

    const Byte* Find(StringView data) const
    {
        auto ret = MaybeFind(data);
        if (!ret) NEPTOOLS_THROW(std::runtime_error{"Couldn't find pattern"});
        return ret;
    }
};

}
#endif
