#ifndef UUID_46683816_A07A_4B9E_8C0B_31A2525F9CE5
#define UUID_46683816_A07A_4B9E_8C0B_31A2525F9CE5
#pragma once

#include "type_traits.hpp"
#include <utility>

namespace Libshit::Lua
{

  template <typename T>
  class AutoTable
  {
  public:
    AutoTable(const AutoTable&) = default;
    AutoTable(AutoTable&&) = default;

    template <typename... Args> AutoTable(Args&&... args)
      : t{std::forward<Args>(args)...} {}

    T& Get() noexcept { return t; }
    const T& Get() const noexcept { return t; }

    operator T&() noexcept { return t; }
    operator const T&() const noexcept { return t; }

  private:
    T t;
  };

#ifndef LIBSHIT_WITHOUT_LUA

  template <typename T>
  using TableCtorPtr = T (*)(StateRef, RawTable);
  template <typename T, typename Enable = void>
  struct GetTableCtor : std::integral_constant<TableCtorPtr<T>, nullptr> {};

  template <typename T>
  inline constexpr TableCtorPtr<T> GET_TABLE_CTOR = GetTableCtor<T>::value;

  template <typename T>
  struct TypeTraits<AutoTable<T>>
  {
    template <bool Unsafe>
    static T Get(StateRef vm, bool arg, int idx)
    {
      if (lua_type(vm, idx) == LUA_TTABLE)
      {
        auto aidx = lua_absindex(vm, idx);
        if constexpr (GET_TABLE_CTOR<T> != nullptr)
          return GET_TABLE_CTOR<T>(vm, aidx); // +1
        else
        {
          auto t = lua_rawgetp(vm, LUA_REGISTRYINDEX, TypeTraits<T>::TAG + 1); // +1
          LIBSHIT_ASSERT(t == LUA_TTABLE); (void) t;
          auto n = vm.Unpack01(aidx); // +1+n
          lua_call(vm, n, 1); // +1
          auto ret = TypeTraits<T>::template Get<false>(vm, arg, -1);
          lua_pop(vm, 1);
          return ret;
        }
      }
      return TypeTraits<T>::template Get<Unsafe>(vm, arg, idx);
    }

    static bool Is(StateRef vm, int idx)
    { return lua_type(vm, idx) == LUA_TTABLE || TypeTraits<T>::Is(vm, idx); }

    static void PrintName(std::ostream& os)
    {
      TypeTraits<T>::PrintName(os);
      os << " or table";
    }

    static constexpr const char* TAG = TypeTraits<T>::TAG;
  };
#endif

}

namespace Libshit
{
  template <typename T> using AT = Lua::AutoTable<T>;
}

#endif
