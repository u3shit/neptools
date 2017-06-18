#ifndef UUID_4999021A_F41A_400B_A951_CDE022AF7331
#define UUID_4999021A_F41A_400B_A951_CDE022AF7331
#pragma once

#include <stdexcept>
#include <typeindex>
#include <boost/exception/info.hpp>

#ifdef NDEBUG
#  define NEPTOOLS_THROW(...) (throw ::boost::enable_error_info(__VA_ARGS__))
#else
#  include "file.hpp"
#  define NEPTOOLS_THROW(...)                               \
    (throw ::boost::enable_error_info(__VA_ARGS__) <<       \
     ::boost::throw_file(NEPTOOLS_FILE) <<                  \
     ::boost::throw_line(__LINE__) <<                       \
     ::boost::throw_function(NEPTOOLS_FUNCTION))
#endif

namespace Neptools
{

BOOST_NORETURN void RethrowBoostException();
std::string ExceptionToString();

#if defined(__GNUC__) && !defined(__clang__)
// gcc mangling is buggy as hell, if two overloads only differ by noexcept, gcc
// will mangle them to the same name, causing assembler (!) errors due to
// duplicate identifiers (I don't want to imagine what happens if the two
// overloads end up in different compilation units...)
#define NEPTOOLS_INVOKE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define NEPTOOLS_INVOKE_ALWAYS_INLINE
#endif

template <typename Base, typename T, typename Derived, typename... Args>
NEPTOOLS_INVOKE_ALWAYS_INLINE
inline decltype(auto) Invoke(T Base::*fun, Derived* thiz, Args&&... args)
    noexcept(noexcept((*thiz.*fun)(std::forward<Args>(args)...)))
{ return (*thiz.*fun)(std::forward<Args>(args)...); }

template <typename Base, typename T, typename Derived, typename... Args>
NEPTOOLS_INVOKE_ALWAYS_INLINE
inline decltype(auto) Invoke(T Base::*fun, Derived& thiz, Args&&... args)
    noexcept(noexcept((thiz.*fun)(std::forward<Args>(args)...)))
{ return (thiz.*fun)(std::forward<Args>(args)...); }

template <typename Fun, typename... Args>
NEPTOOLS_INVOKE_ALWAYS_INLINE
inline decltype(auto) Invoke(Fun&& f, Args&&... args)
    noexcept(noexcept(std::forward<Fun>(f)(std::forward<Args>(args)...)))
{ return std::forward<Fun>(f)(std::forward<Args>(args)...); }

#undef NEPTOOLS_INOKE_ALWAYS_INLINE

template <typename Fun, typename Info, typename... Args>
inline auto AddInfo(Fun fun, Info info_adder, Args&&... args)
{
    try
    {
        try { return Invoke(fun, std::forward<Args>(args)...); }
        catch (const boost::exception& e) { throw; }
        catch (...) { RethrowBoostException(); }
    }
    catch (const boost::exception& e)
    {
        info_adder(e);
        throw;
    }
}
// AddInfo([&](){ ...; }, [&](auto& e) { e << foo; });

#define NEPTOOLS_GEN_EXCEPTION_TYPE(name, base)  \
    struct name : base, virtual boost::exception \
    {                                            \
        using BaseType = base;                   \
        using BaseType::BaseType;                \
    }

NEPTOOLS_GEN_EXCEPTION_TYPE(DecodeError, std::runtime_error);
NEPTOOLS_GEN_EXCEPTION_TYPE(OutOfRange,  std::out_of_range);
NEPTOOLS_GEN_EXCEPTION_TYPE(SystemError, std::system_error);

#define NEPTOOLS_VALIDATE_FIELD(msg, x)                           \
    while (!(x)) NEPTOOLS_THROW(DecodeError{msg": invalid data"}  \
                                << FailedExpression{#x})

using FailedExpression = boost::error_info<struct FailedExpressionTag, const char*>;
using RethrownType = boost::error_info<struct RethrownTypeTag, std::type_index>;

}

namespace std
{
string to_string(const type_index& type);
}

#endif
