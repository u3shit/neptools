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

#include "../except.hpp"

namespace Neptools
{
namespace Lua
{

template <typename T, typename Enable = void> struct TypeTraits;
extern char reftbl;

NEPTOOLS_GEN_EXCEPTION_TYPE(Error, std::runtime_error);

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
    { TypeTraits<std::decay_t<T>>::Push(*this, std::forward<T>(t)); }

    template <typename T, T Fun> void PushFunction();

    void PushFunction(lua_CFunction fun) { lua_pushcfunction(vm, fun); }

    // use optional<T>::value_or to get default value
    template <typename T> boost::optional<T> Opt(int idx)
    {
        if (lua_isnoneornil(vm, idx)) return {};
        return {Check<T>(idx)};
    }

    template <typename T> T Get(int idx = -1)
    { return TypeTraits<T>::Get(*this, false, idx); }
    template <typename T> T Check(int idx)
    { return TypeTraits<T>::Get(*this, true, idx); }

    operator lua_State*() { return vm; }

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

    template <typename T, typename Enable> friend struct TypeTraits;
    BOOST_NORETURN
    void TypeError(bool arg, const char* expected, int idx);
};

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
