#ifndef UUID_04CE9898_AACA_4B50_AC3F_FED6669C33C6
#define UUID_04CE9898_AACA_4B50_AC3F_FED6669C33C6
#pragma once

#include <vector>
#include <lua.hpp>

// what the hell are you doing, boost?
#ifdef _MSC_VER
using std::is_default_constructible;
using std::is_assignable;
#include <excpt.h>
#endif
#include <boost/optional.hpp>

#include "../assert.hpp"
#include "../except.hpp"

#ifdef NDEBUG
#define NEPTOOLS_LUA_GETTOP(vm, name) ((void) 0)
#define NEPTOOLS_LUA_CHECKTOP(vm, val) ((void) 0)
#else
#define NEPTOOLS_LUA_GETTOP(vm, name) auto name = lua_gettop(vm)
#define NEPTOOLS_LUA_CHECKTOP(vm, val) NEPTOOLS_ASSERT(lua_gettop(vm) == val)
#endif

namespace Neptools
{
namespace Lua
{

template <typename T, typename Enable = void> struct TypeTraits;
extern char reftbl;

NEPTOOLS_GEN_EXCEPTION_TYPE(Error, std::runtime_error);

template <typename T, T Fun> struct Overload;
template <typename T> struct IsOverload : public std::false_type {};
template <typename T, T Fun>
struct IsOverload<Overload<T, Fun>> : public std::true_type {};

class StateRef
{
public:
    StateRef(lua_State* vm) : vm{vm} {}

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
        NEPTOOLS_LUA_GETTOP(vm, top);
        TypeTraits<std::decay_t<T>>::Push(*this, std::forward<T>(t));
        NEPTOOLS_LUA_CHECKTOP(vm, top+1);
    }

    template <typename T, T Fun> void Push();
    template <typename Head, typename... Tail>
    typename std::enable_if<IsOverload<Head>::value>::type Push();

    template <typename... Args> void PushAll(Args&&... args)
    { (Push(std::forward<Args>(args)), ...); }

    void Push(lua_CFunction fun) { lua_pushcfunction(vm, fun); }

    // pop table, set table[name] to val at idx; +0 -1
    void SetRecTable(const char* name, int idx = -1);

    // use optional<T>::value_or to get default value
    template <typename T> boost::optional<T> Opt(int idx)
    {
        if (lua_isnoneornil(vm, idx)) return {};
        return {Check<T>(idx)};
    }

    template <typename T> decltype(auto) Get(int idx = -1)
    { return TypeTraits<T>::Get(*this, false, idx); }
    template <typename T> decltype(auto) UnsafeGet(int idx = -1)
    { return TypeTraits<T>::UnsafeGet(*this, idx); }
    template <typename T> decltype(auto) Check(int idx)
    { return TypeTraits<T>::Get(*this, true, idx); }
    template <typename T> bool Is(int idx)
    { return TypeTraits<T>::Is(*this, idx); }


    void DoString(const char* str);

    operator lua_State*() { return vm; }

    BOOST_NORETURN
    void TypeError(bool arg, const char* expected, int idx);

protected:
    lua_State* vm;

private:
#ifdef _MSC_VER
    static int SEHFilter(lua_State* vm, unsigned code);
#else
    void HandleDotdotdotCatch();
#endif
    static thread_local const char* error_msg;
    static thread_local size_t error_len;
};

inline bool IsNoneOrNil(int v) { return v <= 0; }

#define NEPTOOLS_LUA_RUNBC(vm, name)                                    \
    do                                                                  \
    {                                                                   \
        auto runbc_ret = luaL_loadbuffer(                               \
            vm, luaJIT_BC_##name, luaJIT_BC_##name##_SIZE, "neptools"); \
        NEPTOOLS_ASSERT(runbc_ret == 0); (void) runbc_ret;              \
        lua_call(vm, 0, 0);                                             \
    } while (0)


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

//using Traceback = boost::error_info<struct TracebackTag, std::string>;

}
}

#endif
