#ifndef UUID_04CE9898_AACA_4B50_AC3F_FED6669C33C6
#define UUID_04CE9898_AACA_4B50_AC3F_FED6669C33C6
#pragma once

#ifndef LIBSHIT_WITHOUT_LUA

#include "../assert.hpp"
#include "../except.hpp"

#include <vector>
#include <lua.hpp>

// what the hell are you doing, boost?
#ifdef _MSC_VER
using std::is_default_constructible;
using std::is_assignable;
#  include <excpt.h>
#endif
#include <boost/optional.hpp>

#ifdef NDEBUG
#  define LIBSHIT_LUA_GETTOP(vm, name) ((void) 0)
#  define LIBSHIT_LUA_CHECKTOP(vm, val) ((void) 0)
#else
#  define LIBSHIT_LUA_GETTOP(vm, name) auto name = lua_gettop(vm)
#  define LIBSHIT_LUA_CHECKTOP(vm, val) LIBSHIT_ASSERT(lua_gettop(vm) == val)
#endif

namespace Libshit::Lua
{

  template <typename T, typename Enable = void> struct TypeTraits;
  extern char reftbl;

  LIBSHIT_GEN_EXCEPTION_TYPE(Error, std::runtime_error);

  inline bool IsNoneOrNil(int v) { return v <= 0; }

  class StateRef
  {
  public:
    constexpr StateRef(lua_State* vm) noexcept : vm{vm} {}

    template <typename Fun, typename... Args>
    auto Catch(Fun f, Args&&... args)
    {
#ifdef _MSC_VER
      auto vm_ = vm;
      __try { return f(std::forward<Args>(args)...); }
      __except (SEHFilter(vm_, GetExceptionCode()))
      { throw Error({error_msg, error_len}); }
#else
      try { return f(std::forward<Args>(args)...); }
      catch (const std::exception&) { throw; }
      catch (...) { HandleDotdotdotCatch(); }
      throw Error{{error_msg, error_len}};
#endif
    }

    template <typename T> void Push(T&& t)
    {
      LIBSHIT_LUA_GETTOP(vm, top);
      TypeTraits<std::decay_t<T>>::Push(*this, std::forward<T>(t));
      LIBSHIT_LUA_CHECKTOP(vm, top+1);
    }

    template <auto... Funs> void PushFunction();

    template <typename... Args> void PushAll(Args&&... args)
    { (Push(std::forward<Args>(args)), ...); }

    void PushFunction(lua_CFunction fun) { lua_pushcfunction(vm, fun); }

    // pop table, set table[name] to val at idx; +0 -1
    void SetRecTable(const char* name, int idx = -1);

    // use optional<T>::value_or to get default value
    template <typename T> boost::optional<T> Opt(int idx)
    {
      if (lua_isnoneornil(vm, idx)) return {};
      return {Check<T>(idx)};
    }

    template <typename T, bool Unsafe = false> decltype(auto) Get(int idx = -1)
    { return TypeTraits<T>::template Get<Unsafe>(*this, false, idx); }

    template <typename T, bool Unsafe = false> decltype(auto) Check(int idx)
    { return TypeTraits<T>::template Get<Unsafe>(*this, true, idx); }

    template <typename T> bool Is(int idx)
    { return TypeTraits<T>::Is(*this, idx); }

    const char* TypeName(int idx);
    void DoString(const char* str);

    constexpr operator lua_State*() noexcept { return vm; }

    BOOST_NORETURN
    void TypeError(bool arg, const char* expected, int idx);
    BOOST_NORETURN
    void GetError(bool arg, int idx, const char* msg);

    struct RawLen01Ret { size_t len; bool one_based; };
    RawLen01Ret RawLen01(int idx);
    template <typename Fun>
    void Ipairs01(int idx, Fun f)
    {
      auto [i, type] = Ipairs01Prep(idx);
      while (!IsNoneOrNil(type))
      {
        LIBSHIT_LUA_GETTOP(vm, top);
        f(i, type);
        LIBSHIT_LUA_CHECKTOP(vm, top);

        lua_pop(vm, 1); // 0
        type = lua_rawgeti(vm, idx, ++i); // +1
      }
      lua_pop(vm, 1); // 0
    }
    template <typename Fun>
    void Fori(int idx, size_t offset, size_t len, Fun f)
    {
      for (size_t i = 0; i < len; ++i)
      {
        LIBSHIT_LUA_GETTOP(vm, top);
        f(i, lua_rawgeti(vm, idx, i + offset));
        lua_pop(vm, 1);
        LIBSHIT_LUA_CHECKTOP(vm, top);
      }
    }
    size_t Unpack01(int idx); // +ret

  protected:
    lua_State* vm;

  private:
    std::pair<size_t, int> Ipairs01Prep(int idx);

#ifdef _MSC_VER
    static int SEHFilter(lua_State* vm, unsigned code);
#else
    void HandleDotdotdotCatch();
#endif
    static thread_local const char* error_msg;
    static thread_local size_t error_len;
  };

#define LIBSHIT_LUA_RUNBC(vm, name, retnum)                  \
  do                                                         \
  {                                                          \
    auto runbc_ret = luaL_loadbuffer(                        \
      vm, luaJIT_BC_##name, luaJIT_BC_##name##_SIZE, #name); \
    LIBSHIT_ASSERT(runbc_ret == 0); (void) runbc_ret;        \
    lua_call(vm, 0, retnum);                                 \
  } while (false)


  class State final : public StateRef
  {
    State(int dummy);
  public:
    State();
    ~State();
    State(const State&) = delete;
    void operator=(const State&) = delete;

    using RegisterFun = void (*)(StateRef);
  private:
    static auto& Registers()
    {
      static std::vector<RegisterFun> registers;
      return registers;
    }
  public:
    struct Register
    {
      Register(RegisterFun fun) { Registers().push_back(fun); }
    };
  };
}

#endif
#endif
