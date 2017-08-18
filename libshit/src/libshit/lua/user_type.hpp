#ifndef UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#define UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#pragma once

#include "user_type_fwd.hpp"

#ifndef LIBSHIT_WITHOUT_LUA

#include "function_call.hpp"
#include "type_traits.hpp"
#include "../shared_ptr.hpp"

#include <type_traits>

namespace Libshit { template <typename T> class NotNull; }

namespace Libshit::Lua
{

  // When specifying T as template argument for vararg &&... argument,
  // std::forward will try to convert it to T&&. If TypeTraits::Get returns a
  // T&, it'll fail, even though normally it'd copy it. Thus we need to specify
  // T& in this case (or const T&).
  namespace Detail
  {
    template <typename T, typename = void> struct LuaGetRefHlp
    {
      using X = decltype(TypeTraits<T>::template Get<false>(
                           std::declval<StateRef>(), false, 0));
      using Type = std::conditional_t<
        !std::is_reference_v<X> || std::is_rvalue_reference_v<X>, T, T&>;
    };

    template <typename T>
    struct LuaGetRefHlp<T, std::enable_if_t<std::is_reference<T>::value>>
    { using Type = T; };

    template <typename T>
    struct LuaGetRefHlp<T, EnableIfTupleLike<T>> { using Type = T; };

    template <> struct LuaGetRefHlp<StateRef> { using Type = StateRef; };
    template <> struct LuaGetRefHlp<Skip>     { using Type = Skip; };
    template <> struct LuaGetRefHlp<VarArg>   { using Type = VarArg; };
    template <> struct LuaGetRefHlp<Any>      { using Type = Any; };
    template <int I> struct LuaGetRefHlp<Raw<I>> { using Type = Raw<I>; };
  }

  template <typename T>
  using LuaGetRef = typename Detail::LuaGetRefHlp<T>::Type;

  template <typename Class, typename T, T Class::* Member>
  BOOST_FORCEINLINE
  T& GetMember(Class& cls) { return cls.*Member; }

  template <typename Class, typename T, T Class::* member>
  BOOST_FORCEINLINE
  void SetMember(Class& cls, const T& val) { cls.*member = val; }

  class TypeBuilder
  {
  public:
    TypeBuilder(StateRef vm, const char* name, bool instantiable);
    TypeBuilder(const TypeBuilder&) = delete;
    void operator=(const TypeBuilder&) = delete;

    StateRef GetVm() noexcept { return vm; }
    operator lua_State*() { return static_cast<lua_State*>(vm); }

    template <typename T>
    void Init()
    {
      using UT = UserTypeTraits<T>;
      if constexpr (UT::NEEDS_GC)
        AddFunction<UT::GcFun>("__gc");
    }

    void Done();

    template <typename Deriv, typename... Base>
    void Inherit() { InheritHelp<Deriv, Base...>::Do(*this); }

    template <auto... Funs>
    void AddFunction(const char* name)
    {
      vm.PushFunction<Funs...>();
      SetField(name);
    }

    template <typename T>
    void Add(const char* name, T&& t)
    {
      vm.Push(std::forward<T>(t));
      SetField(name);
    }

    // low-level, pops value from lua stack
    void SetField(const char* name)
    {
      LIBSHIT_LUA_GETTOP(vm, top);

      lua_pushvalue(vm, -1);
      lua_setfield(vm, -4, name);
      lua_setfield(vm, -2, name);

      LIBSHIT_LUA_CHECKTOP(vm, top-1);
    }

    void TaggedNew();

  private:
    template <typename Deriv, typename... Base>
    struct InheritHelp;

    static int IsFunc(lua_State* vm) noexcept;

    void DoInherit(ptrdiff_t offs);

    StateRef vm;
    bool instantiable;
  };

  class TypeRegister
  {
  public:
    template <typename Class>
    static void Register(StateRef vm)
    {
      LIBSHIT_LUA_GETTOP(vm, top);

      bool doit = true;
      if constexpr (UserTypeTraits<Class>::INSTANTIABLE)
      {
        lua_rawgetp(vm, LUA_REGISTRYINDEX, TYPE_NAME<Class>); // +1
        doit = lua_isnil(vm, -1);
        if (doit) lua_pop(vm, 1);
      }
      if (doit)
      {
        TypeBuilder bld{
          vm, TYPE_NAME<Class>, UserTypeTraits<Class>::INSTANTIABLE};
        bld.Init<Class>();
        TypeRegisterTraits<Class>::Register(bld);
        bld.Done();
      }

      LIBSHIT_LUA_CHECKTOP(vm, top+1);
    }

    template <typename... Args>
    static void MultiRegister(StateRef vm)
    {
      using Swallow = int[];
      (void) Swallow{ 0, (Register<Args>(vm), lua_pop(vm, 1), 0)... };
    }

    template <typename... Classes>
    struct StateRegister : State::Register
    {
      StateRegister() : Register{&TypeRegister::MultiRegister<Classes...>} {}
    };
  };

  template <typename Deriv>
  struct TypeBuilder::InheritHelp<Deriv>
  { static void Do(TypeBuilder&) {} };

  template <typename Deriv, typename Head, typename... Rest>
  struct TypeBuilder::InheritHelp<Deriv, Head, Rest...>
  {
    static void Do(TypeBuilder& bld)
    {
      TypeRegister::Register<Head>(bld.vm);
      char* c = reinterpret_cast<char*>(1);
      bld.DoInherit(
        reinterpret_cast<char*>(static_cast<Head*>(reinterpret_cast<Deriv*>(c)))-c);
      InheritHelp<Deriv, Rest...>::Do(bld);
    }
  };

}

#endif
#endif
