#ifndef UUID_8BEC2FCA_66B2_48E5_A689_E29590320829
#define UUID_8BEC2FCA_66B2_48E5_A689_E29590320829
#pragma once

#include "lua/user_type.hpp"

#include "dynamic_struct.hpp"
#include <type_traits>
#include <boost/integer.hpp>
#include <boost/hana/maximum.hpp>
#include <boost/hana/tuple.hpp>

namespace Neptools
{

namespace Detail
{

template <typename T, size_t N, typename... Args> struct IndexOfImpl;

template <typename T, size_t N, typename... Args>
struct IndexOfImpl<T, N, T, Args...>
{ static constexpr size_t VALUE = N; };

template <typename T, size_t N, typename U, typename... Args>
struct IndexOfImpl<T, N, U, Args...>
{ static constexpr size_t VALUE = IndexOfImpl<T,N+1,Args...>::VALUE; };

template <typename T, typename... Args>
constexpr size_t IndexOf = IndexOfImpl<T, 0, Args...>::VALUE;

template <typename T>
struct DynamicStructTypeTraitsName
{
    static constexpr const char* NAME = Lua::TYPE_NAME<T>;
};

}

template <typename T, typename Enable = void>
struct DynamicStructTypeTraits : Detail::DynamicStructTypeTraitsName<T>
{
    static void Push(Lua::StateRef vm, const void* ptr, size_t size)
    {
        NEPTOOLS_ASSERT(size == sizeof(T)); (void) size;
        vm.Push(*static_cast<const T*>(ptr));
    }

    static void Get(Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        NEPTOOLS_ASSERT(size == sizeof(T)); (void) size;
        *static_cast<T*>(ptr) = vm.Check<T>(idx);
    }

    static constexpr bool SIZABLE = false;
};

// do not use standard lua names for them since they're only "integer"/"number"
// as far as lua is concerned
#define CNAME(type, name) \
    template<> struct Detail::DynamicStructTypeTraitsName<type> \
    { static constexpr const char* NAME = name; }
CNAME(int8_t,   "int8");   CNAME(int16_t,  "int16");
CNAME(int32_t,  "int32");  CNAME(int64_t,  "int64");
CNAME(uint8_t,  "uint8");  CNAME(uint16_t, "uint16");
CNAME(uint32_t, "uint32"); CNAME(uint64_t, "uint64");
CNAME(float,    "float");  CNAME(double,   "double");
#undef CNAME

template <typename... Args>
struct DynamicStructTypeInfo
{
    void (*push)(Lua::StateRef vm, const void* ptr, size_t size);
    void (*get)(Lua::StateRef vm, int idx, void* ptr, size_t size);
    const char* name;

    typename boost::uint_value_t<sizeof...(Args)-1>::least index;
    typename boost::uint_value_t<
        boost::hana::maximum(boost::hana::tuple_c<size_t, sizeof(Args)...>).value
    >::least size;
    bool sizable;
};

template <typename... Args>
inline constexpr const DynamicStructTypeInfo<Args...> infos[sizeof...(Args)] = {
    {
        &DynamicStructTypeTraits<Args>::Push,
        &DynamicStructTypeTraits<Args>::Get,
        DynamicStructTypeTraits<Args>::NAME,
        Detail::IndexOf<Args, Args...>,
        sizeof(Args),
        DynamicStructTypeTraits<Args>::SIZABLE,
    }...,
};

template <typename... Args>
struct DynamicStructBuilderLua
{
    using FakeClass = typename DynamicStruct<Args...>::TypeBuilder;

    NEPTOOLS_NOLUA
    static const DynamicStructTypeInfo<Args...>& GetInfo(Lua::StateRef vm)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);

        int r = lua_rawgetp(vm, LUA_REGISTRYINDEX, &infos<Args...>); //+1
        NEPTOOLS_ASSERT(r);
        lua_pushvalue(vm, 2); //+2
        r = lua_rawget(vm, -2); //+2
        if (Lua::IsNoneOrNil(r))
            luaL_error(vm, "Invalid type %s", vm.Get<const char*>(2));

        NEPTOOLS_ASSERT(r == LUA_TLIGHTUSERDATA);
        auto ret = lua_touserdata(vm, -1);
        NEPTOOLS_ASSERT(ret);
        lua_pop(vm, 2); //+0

        NEPTOOLS_LUA_CHECKTOP(vm, top);
        return *static_cast<DynamicStructTypeInfo<Args...>*>(ret);
    }

    // bld:add(type, size) -> bld
    static Lua::RetNum Add(
        Lua::StateRef vm, FakeClass& bld, Lua::Raw<LUA_TSTRING>, size_t size)
    {
        auto& t = GetInfo(vm);
        if (!t.sizable && t.size != size)
            luaL_error(vm, "Type %s is not sizable", t.name);
        bld.Add(t.index, size);

        lua_pushvalue(vm, 1);
        return 1;
    }

    // bld:add(type) -> bld
    static Lua::RetNum Add(
        Lua::StateRef vm, FakeClass& bld, Lua::Raw<LUA_TSTRING>)
    {
        auto& t = GetInfo(vm);
        if (t.sizable)
            luaL_error(vm, "Type %s requires size", t.name);
        bld.Add(t.index, t.size);

        lua_pushvalue(vm, 1);
        return 1;
    }
};

template <typename... Args>
struct DynamicStructTypeLua
{
    using FakeClass = typename DynamicStruct<Args...>::Type;

    // type[i] -> {type=string,size=int}|nil
    static Lua::RetNum Get(Lua::StateRef vm, const FakeClass& t, size_t i) noexcept
    {
        NEPTOOLS_LUA_GETTOP(vm, top);
        if (i >= t.item_count)
        {
            lua_pushnil(vm); // +1
            NEPTOOLS_LUA_CHECKTOP(vm, top+1);
            return 1;
        }
        NEPTOOLS_ASSERT(t.items[i].idx < sizeof...(Args));

        const auto& info = infos<Args...>[t.items[i].idx];
        lua_createtable(vm, 0, 2); // +1
        lua_pushstring(vm, info.name); // +2
        lua_setfield(vm, -2, "type"); // +1

        lua_pushinteger(vm, info.size); // +2
        lua_setfield(vm, -2, "size"); // +1

        NEPTOOLS_LUA_CHECKTOP(vm, top+1);
        return 1;
    }

    static void Get() noexcept {}
};

template <typename... Args>
struct DynamicStructLua
{
    using FakeClass = DynamicStruct<Args...>; // must be first
    static_assert(sizeof...(Args) > 0);

    static Lua::RetNum Get(Lua::StateRef vm, const FakeClass& s, size_t i) noexcept
    {
        if (i >= s.GetSize())
        {
            lua_pushnil(vm);
            return 1;
        }
        auto idx = s.GetTypeIndex(i);
        NEPTOOLS_ASSERT(idx < sizeof...(Args));
        infos<Args...>[idx].push(vm, s.GetData(i), s.GetSize(i));
        return 1;
    }
    static void Get() noexcept {}

    static void Set(Lua::StateRef vm, FakeClass& s, size_t i)
    {
        if (i >= s.GetSize())
            NEPTOOLS_THROW(std::out_of_range{"DynamicStruct"});
        auto idx = s.GetTypeIndex(i);
        NEPTOOLS_ASSERT(idx < sizeof...(Args));
        infos<Args...>[idx].get(vm, 3, s.GetData(i), s.GetSize(i));
    }

    static Lua::RetNum ToTable(Lua::StateRef vm, FakeClass& s)
    {
        auto size = s.GetSize();
        lua_createtable(vm, size ? size-1 : size, 0); // +1
        for (size_t i = 0; i < size; ++i)
        {
            auto idx = s.GetTypeIndex(i);
            NEPTOOLS_ASSERT(idx < sizeof...(Args));
            infos<Args...>[idx].push(vm, s.GetData(i), s.GetSize(i)); // +2
            lua_rawseti(vm, -2, i); // +1
        }
        return 1;
    }

    NEPTOOLS_NOLUA static void Register(Lua::TypeBuilder& bld)
    {
        // create type table
        lua_createtable(bld, sizeof...(Args)-1, 0); //+1

        for (size_t i = 0; i < sizeof...(Args); ++i)
        {
            lua_pushlightuserdata(
                bld, implicit_const_cast<void*>(&infos<Args...>[i])); //+2
            lua_setfield(bld, -2, infos<Args...>[i].name); //+1
        }
        lua_rawsetp(bld, LUA_REGISTRYINDEX, infos<Args...>); //+0

        luaL_getmetatable(bld, "neptools_ipairs");
        bld.SetField("__ipairs");
    }
};

}

#define NEPTOOLS_DYNAMIC_STRUCT_LUAGEN(name, ...)                               \
    template class ::Neptools::DynamicStruct<__VA_ARGS__>::TypeBuilder;         \
    template struct ::Neptools::DynamicStructLua<__VA_ARGS__>;                  \
    template struct ::Neptools::DynamicStructBuilderLua<__VA_ARGS__>;           \
    template struct ::Neptools::DynamicStructTypeLua<__VA_ARGS__>;              \
    NEPTOOLS_LUA_TEMPLATE(DynStructBind##name, #name,                           \
                          ::Neptools::DynamicStruct<__VA_ARGS__>);              \
    NEPTOOLS_LUA_TEMPLATE(DynStructTypeBind##name, #name..".type",              \
                          ::Neptools::DynamicStruct<__VA_ARGS__>::Type);        \
    NEPTOOLS_LUA_TEMPLATE(DynStructBldBind##name, #name..".builder",            \
                          ::Neptools::DynamicStruct<__VA_ARGS__>::TypeBuilder)

#endif
