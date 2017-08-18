#ifndef UUID_D13BEDEB_B9E0_4D88_A07E_03C11564BE1C
#define UUID_D13BEDEB_B9E0_4D88_A07E_03C11564BE1C
#pragma once

#include "value.hpp"

namespace Libshit::Lua
{

  template <typename Value = StackValue>
  class FunctionRef
  {
  public:
    template <typename... Args>
    FunctionRef(Args&&... args) : fun{std::forward<Args>(args)...} {}

    template <typename Ret, typename... Args>
    Ret Call(StateRef vm, Args&&... args)
    {
      LIBSHIT_LUA_GETTOP(vm, top);

      fun.Push(vm);
      LIBSHIT_ASSERT(lua_type(vm, -1) == LUA_TFUNCTION);
      vm.PushAll(std::forward<Args>(args)...);
      lua_call(vm, sizeof...(Args), 1);

      auto ret = vm.Get<Ret>();
      lua_pop(vm, 1);
      LIBSHIT_LUA_CHECKTOP(vm, top);
      return ret;
    }

  private:
    Value fun;
  };

  template <typename Ret, typename Value = StackValue>
  class FunctionWrapBase : public FunctionRef<Value>
  {
  public:
    template <typename... Args>
    FunctionWrapBase(StateRef vm, Args&&... args)
      : FunctionRef<Value>{vm, std::forward<Args>(args)...}, vm{vm} {}

  protected:
    StateRef vm;
  };

  template <typename Ret, typename Value = StackValue>
  struct FunctionWrapGen : public FunctionWrapBase<Ret, Value>
  {
    using FunctionWrapBase<Ret, Value>::FunctionWrapBase;

    template <typename... Args>
    decltype(auto) operator()(Args&&... args)
    { return this->template Call<Ret>(this->vm, std::forward<Args>(args)...); }
  };

  template <typename Func, typename Value = StackValue> struct FunctionWrap;

  template <typename Ret, typename... Args, typename Value>
  struct FunctionWrap<Ret(Args...), Value> : public FunctionWrapBase<Ret, Value>
  {
    using FunctionWrapBase<Ret, Value>::FunctionWrapBase;

    decltype(auto) operator()(Args... args)
    { return this->template Call<Ret>(this->vm, args...); }
  };

  template <typename T> struct IsFunctionWrap : std::false_type {};
  template <typename Ret, typename Value>
  struct IsFunctionWrap<FunctionWrapGen<Ret, Value>> : std::true_type {};
  template <typename Fun, typename Value>
  struct IsFunctionWrap<FunctionWrap<Fun, Value>> : std::true_type {};

  // todo: this only works with StackValue
  template <typename T>
  struct TypeTraits<T, std::enable_if_t<IsFunctionWrap<T>::value>>
  {
    template <bool Unsafe>
    static T Get(StateRef vm, bool arg, int idx)
    {
      if (Unsafe || BOOST_LIKELY(lua_isfunction(vm, idx))) return {vm, idx};
      vm.TypeError(arg, "function", idx);
    }

    static bool Is(StateRef vm, int idx) { return lua_isfunction(vm, idx); }
    static void PrintName(std::ostream& os) { os << "function"; }
  };

}

#endif
