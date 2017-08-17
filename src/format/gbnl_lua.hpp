#ifndef UUID_907F2DFB_AA53_4295_B41F_9AAA950AEEB4
#define UUID_907F2DFB_AA53_4295_B41F_9AAA950AEEB4
#pragma once

#include "gbnl.hpp"
#include "../dynamic_struct.lua.hpp"

#ifndef LIBSHIT_WITHOUT_LUA

namespace Neptools
{

template <>
struct DynamicStructTypeTraits<Gbnl::OffsetString>
{
    static void Push(Libshit::Lua::StateRef vm, const void* ptr, size_t size)
    {
        LIBSHIT_ASSERT(size == sizeof(Gbnl::OffsetString)); (void) size;
        auto ofs = static_cast<const Gbnl::OffsetString*>(ptr);
        if (ofs->offset == static_cast<uint32_t>(-1))
            lua_pushnil(vm);
        else
            vm.Push(ofs->str);
    }

    static void Get(Libshit::Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        LIBSHIT_ASSERT(size == sizeof(Gbnl::OffsetString)); (void) size;
        auto ofs = static_cast<Gbnl::OffsetString*>(ptr);

        if (Libshit::Lua::IsNoneOrNil(lua_type(vm, idx)))
        {
            ofs->offset = static_cast<uint32_t>(-1);
            ofs->str.clear();
        }
        else
        {
            ofs->str = vm.Check<std::string>(idx);
            ofs->offset = 0; // no longer null
        }
    }

    static constexpr bool SIZABLE = false;
    static constexpr const char* NAME = "string";
};

// FixString is zero terminated, Padding is not
template <>
struct DynamicStructTypeTraits<Gbnl::FixStringTag>
{
    static void Push(Libshit::Lua::StateRef vm, const void* ptr, size_t size)
    {
        auto str = static_cast<const char*>(ptr);
        lua_pushlstring(vm, str, strnlen(str, size));
    }

    static void Get(Libshit::Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        auto str = vm.Check<Libshit::StringView>(idx);
        auto dst = static_cast<char*>(ptr);

        auto n = std::min(size-1, str.length());
        memcpy(dst, str.data(), n);
        memset(dst+n, 0, size-n);
    }

    static constexpr bool SIZABLE = true;
    static constexpr const char* NAME = "fix_string";
};

template<>
struct DynamicStructTypeTraits<Gbnl::PaddingTag>
{
    static void Push(Libshit::Lua::StateRef vm, const void* ptr, size_t size)
    {
        auto str = static_cast<const char*>(ptr);
        lua_pushlstring(vm, str, size);
    }

    static void Get(Libshit::Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        auto str = vm.Check<Libshit::StringView>(idx);
        auto dst = static_cast<char*>(ptr);

        auto n = std::min(size, str.length());
        memcpy(dst, str.data(), n);
        memset(dst+n, 0, size-n);
    }

    static constexpr bool SIZABLE = true;
    static constexpr const char* NAME = "padding";
};

}

NEPTOOLS_DYNAMIC_STRUCT_TABLECTOR(
    int8_t, int16_t, int32_t, int64_t, float,
    ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag,
    ::Neptools::Gbnl::PaddingTag);

#endif
#endif
