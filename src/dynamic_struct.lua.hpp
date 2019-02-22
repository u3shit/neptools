#ifndef UUID_8BEC2FCA_66B2_48E5_A689_E29590320829
#define UUID_8BEC2FCA_66B2_48E5_A689_E29590320829
#pragma once

#if !LIBSHIT_WITH_LUA
#define NEPTOOLS_DYNAMIC_STRUCT_LUAGEN(name, ...)
#define NEPTOOLS_DYNAMIC_STRUCT_TABLECTOR(...)
#else

#include "dynamic_struct.hpp"

#include <libshit/lua/auto_table.hpp>
#include <libshit/lua/user_type.hpp>
#include <libshit/utils.hpp>

#include <type_traits>
#include <boost/integer.hpp>

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
      static constexpr const char* NAME = Libshit::Lua::TYPE_NAME<T>;
    };

  }

  template <typename T, typename Enable = void>
  struct DynamicStructTypeTraits : Detail::DynamicStructTypeTraitsName<T>
  {
    static void Push(Libshit::Lua::StateRef vm, const void* ptr, size_t size)
    {
      LIBSHIT_ASSERT(size == sizeof(T)); (void) size;
      vm.Push(*static_cast<const T*>(ptr));
    }

    static void Get(Libshit::Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
      LIBSHIT_ASSERT(size == sizeof(T)); (void) size;
      *static_cast<T*>(ptr) = vm.Check<T>(idx);
    }

    static constexpr bool SIZABLE = false;
  };

  // do not use standard lua names for them since they're only "integer"/"number"
  // as far as lua is concerned
#define CNAME(type, name)                                       \
  template<> struct Detail::DynamicStructTypeTraitsName<type>   \
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
    void (*push)(Libshit::Lua::StateRef vm, const void* ptr, size_t size);
    void (*get)(Libshit::Lua::StateRef vm, int idx, void* ptr, size_t size);
    const char* name;

    typename boost::uint_value_t<sizeof...(Args)-1>::least index;
    typename boost::uint_value_t<std::max({sizeof(Args)...})>::least size;
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

    LIBSHIT_NOLUA
    static const DynamicStructTypeInfo<Args...>&
    GetInfo(Libshit::Lua::StateRef vm, Libshit::Lua::Raw<LUA_TSTRING> name)
    {
      LIBSHIT_LUA_GETTOP(vm, top);

      int r = lua_rawgetp(vm, LUA_REGISTRYINDEX, &infos<Args...>); //+1
      LIBSHIT_ASSERT(r);
      lua_pushvalue(vm, name); //+2
      r = lua_rawget(vm, -2); //+2
      if (Libshit::Lua::IsNoneOrNil(r))
        luaL_error(vm, "Invalid type %s", vm.Get<const char*, true>(name));

      LIBSHIT_ASSERT(r == LUA_TLIGHTUSERDATA);
      auto ret = lua_touserdata(vm, -1);
      LIBSHIT_ASSERT(ret);
      lua_pop(vm, 2); //+0

      LIBSHIT_LUA_CHECKTOP(vm, top);
      return *static_cast<DynamicStructTypeInfo<Args...>*>(ret);
    }

    // bld:add(type, size) -> bld
    static Libshit::Lua::RetNum Add(
      Libshit::Lua::StateRef vm,
      typename DynamicStruct<Args...>::TypeBuilder& bld,
      Libshit::Lua::Raw<LUA_TSTRING> name, size_t size)
    {
      LIBSHIT_LUA_GETTOP(vm, top);
      auto& t = GetInfo(vm, name);
      if (!t.sizable && t.size != size)
        luaL_error(vm, "Type %s is not sizable", t.name);
      bld.Add(t.index, size);

      lua_pushvalue(vm, 1);
      LIBSHIT_LUA_CHECKTOP(vm, top+1);
      return 1;
    }

    // bld:add(type) -> bld
    static Libshit::Lua::RetNum Add(
      Libshit::Lua::StateRef vm,
      typename DynamicStruct<Args...>::TypeBuilder& bld,
      Libshit::Lua::Raw<LUA_TSTRING> name)
    {
      LIBSHIT_LUA_GETTOP(vm, top);
      auto& t = GetInfo(vm, name);
      if (t.sizable)
        luaL_error(vm, "Type %s requires size", t.name);
      bld.Add(t.index, t.size);

      lua_pushvalue(vm, 1);
      LIBSHIT_LUA_CHECKTOP(vm, top+1);
      return 1;
    }
  };

  template <typename... Args>
  struct DynamicStructTypeLua
  {
    using FakeClass = typename DynamicStruct<Args...>::Type;
    using Builder = typename DynamicStruct<Args...>::TypeBuilder;
    using BuilderLua = DynamicStructBuilderLua<Args...>;

    // type[i] -> {type=string,size=int}|nil
    static Libshit::Lua::RetNum Get(
      Libshit::Lua::StateRef vm,
      const typename DynamicStruct<Args...>::Type& t,
      size_t i) noexcept
    {
      LIBSHIT_LUA_GETTOP(vm, top);
      if (i >= t.item_count)
      {
        lua_pushnil(vm); // +1
        LIBSHIT_LUA_CHECKTOP(vm, top+1);
        return 1;
      }
      LIBSHIT_ASSERT(t.items[i].idx < sizeof...(Args));

      const auto& info = infos<Args...>[t.items[i].idx];
      lua_createtable(vm, 0, 2); // +1
      lua_pushstring(vm, info.name); // +2
      lua_setfield(vm, -2, "type"); // +1

      lua_pushinteger(vm, info.size); // +2
      lua_setfield(vm, -2, "size"); // +1

      LIBSHIT_LUA_CHECKTOP(vm, top+1);
      return 1;
    }

    static void Get(
      const typename DynamicStruct<Args...>::Type&,
      Libshit::Lua::VarArg) noexcept {}

    LIBSHIT_NOLUA
    // {"name",size} or {name="name",size=size}
    // size optional
    static void AddTableType(Libshit::Lua::StateRef vm, Builder& bld)
    {
      LIBSHIT_LUA_GETTOP(vm, top);
      int size_type = LUA_TSTRING; // whatever, just be invalid
      if (lua_rawgeti(vm, -1, 1) == LUA_TSTRING) // +1
        size_type = lua_rawgeti(vm, -2, 2); // +2
      else if (lua_pop(vm, 1); lua_getfield(vm, -1, "name") == LUA_TSTRING) // +1
        size_type = lua_getfield(vm, -2, "size"); // +2

      if (Libshit::Lua::IsNoneOrNil(size_type))
        BuilderLua::Add(vm, bld, {lua_absindex(vm, -2)}); // +3
      else if (size_type == LUA_TNUMBER)
        BuilderLua::Add(vm, bld, {lua_absindex(vm, -2)},
                        vm.Get<int, true>(-1)); // +3
      else
        luaL_error(vm, "invalid type table, expected {string,integer} or "
                   "{name=string, size=integer}");

      lua_pop(vm, 2);
      LIBSHIT_LUA_CHECKTOP(vm, top+1);
    }

    // create from table
    static boost::intrusive_ptr<const FakeClass>
    New(Libshit::Lua::StateRef vm, Libshit::Lua::RawTable tbl)
    {
      Builder bld;
      auto [len, one] = vm.RawLen01(tbl);
      bld.Reserve(len);
      vm.Fori(tbl, one, len, [&](size_t, int type)
      {
        if (type == LUA_TSTRING)
          BuilderLua::Add(vm, bld, {lua_absindex(vm, -1)}); // +1
        else if (type == LUA_TTABLE)
          AddTableType(vm, bld); // +1
        else
          vm.TypeError(false, "string or table", -1);

        lua_pop(vm, 1); // 0
      });

      return bld.Build();
    }
  };

  template <typename... Args>
  struct DynamicStructLua
  {
    using FakeClass = DynamicStruct<Args...>; // must be first
    static_assert(sizeof...(Args) > 0);

    static Libshit::Lua::RetNum Get(
      Libshit::Lua::StateRef vm, const DynamicStruct<Args...>& s,
      size_t i) noexcept
    {
      if (i >= s.GetSize())
      {
        lua_pushnil(vm);
        return 1;
      }
      auto idx = s.GetTypeIndex(i);
      LIBSHIT_ASSERT(idx < sizeof...(Args));
      infos<Args...>[idx].push(vm, s.GetData(i), s.GetSize(i));
      return 1;
    }
    static void Get(
      const DynamicStruct<Args...>&, Libshit::Lua::VarArg) noexcept {}

    static void Set(
      Libshit::Lua::StateRef vm, DynamicStruct<Args...>& s, size_t i,
      Libshit::Lua::Any val)
    {
      if (i >= s.GetSize())
        LIBSHIT_THROW(std::out_of_range, "DynamicStruct");
      auto idx = s.GetTypeIndex(i);
      LIBSHIT_ASSERT(idx < sizeof...(Args));
      infos<Args...>[idx].get(vm, val, s.GetData(i), s.GetSize(i));
    }

    static Libshit::Lua::RetNum ToTable(
      Libshit::Lua::StateRef vm, DynamicStruct<Args...>& s)
    {
      auto size = s.GetSize();
      lua_createtable(vm, size ? size-1 : size, 0); // +1
      for (size_t i = 0; i < size; ++i)
      {
        auto idx = s.GetTypeIndex(i);
        LIBSHIT_ASSERT(idx < sizeof...(Args));
        infos<Args...>[idx].push(vm, s.GetData(i), s.GetSize(i)); // +2
        lua_rawseti(vm, -2, i); // +1
      }
      return 1;
    }

    static boost::intrusive_ptr<FakeClass> New(
      Libshit::Lua::StateRef vm, const typename FakeClass::TypePtr type,
      Libshit::Lua::RawTable vals)
    {
      auto s = FakeClass::New(type);
      size_t i = 0;
      vm.Ipairs01(vals, [&](size_t, int)
                  { Set(vm, *s, i++, {lua_absindex(vm, -1)}); });
      return s;
    }

    LIBSHIT_NOLUA static void Register(Libshit::Lua::TypeBuilder& bld)
    {
      // create type table
      lua_createtable(bld, sizeof...(Args)-1, 0); //+1

      for (size_t i = 0; i < sizeof...(Args); ++i)
      {
        lua_pushlightuserdata(
          bld, Libshit::implicit_const_cast<void*>(&infos<Args...>[i])); //+2
        lua_setfield(bld, -2, infos<Args...>[i].name); //+1
      }
      lua_rawsetp(bld, LUA_REGISTRYINDEX, infos<Args...>); //+0

      luaL_getmetatable(bld, "libshit_ipairs");
      bld.SetField("__ipairs");
    }
  };

}

#define NEPTOOLS_DYNAMIC_STRUCT_TABLECTOR(...)                                  \
  /* workaround can't specialize for nested classes in template class, because  \
     well, including a wrapper around cairo in the standard is more important   \
     than fixing problems like this */                                          \
  template<> struct Libshit::Lua::GetTableCtor<                                 \
    boost::intrusive_ptr<const ::Neptools::DynamicStruct<__VA_ARGS__>::Type>>   \
    : std::integral_constant<::Libshit::Lua::TableCtorPtr<                      \
        boost::intrusive_ptr<                                                   \
          const ::Neptools::DynamicStruct<__VA_ARGS__>::Type>>,                 \
          ::Neptools::DynamicStructTypeLua<__VA_ARGS__>::New> {}
#define NEPTOOLS_DYNAMIC_STRUCT_LUAGEN(nam, ...)                                \
  template class ::Neptools::DynamicStruct<__VA_ARGS__>::TypeBuilder;           \
  template struct ::Neptools::DynamicStructLua<__VA_ARGS__>;                    \
  template struct ::Neptools::DynamicStructBuilderLua<__VA_ARGS__>;             \
  template struct ::Neptools::DynamicStructTypeLua<__VA_ARGS__>;                \
  template<> struct Libshit::Lua::GetTableCtor<                                 \
    ::Libshit::NotNull<boost::intrusive_ptr<::Neptools::DynamicStruct<__VA_ARGS__>>>> \
    : std::integral_constant<::Libshit::Lua::TableCtorPtr<                      \
        ::Libshit::NotNull<boost::intrusive_ptr<::Neptools::DynamicStruct<__VA_ARGS__>>>>, \
        nullptr> {};                                                            \
  LIBSHIT_LUA_TEMPLATE(DynStructBind##nam, (name=#nam),                         \
                       ::Neptools::DynamicStruct<__VA_ARGS__>);                 \
  LIBSHIT_LUA_TEMPLATE(DynStructTypeBind##nam, (name=#nam..".type"),            \
                       ::Neptools::DynamicStruct<__VA_ARGS__>::Type);           \
  LIBSHIT_LUA_TEMPLATE(DynStructBldBind##nam, (name=#nam..".builder"),          \
                       ::Neptools::DynamicStruct<__VA_ARGS__>::TypeBuilder)

#endif
#endif
