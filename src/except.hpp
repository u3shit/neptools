#ifndef UUID_4999021A_F41A_400B_A951_CDE022AF7331
#define UUID_4999021A_F41A_400B_A951_CDE022AF7331
#pragma once

#include <stdexcept>
#include <typeindex>
#include <boost/exception/info.hpp>

#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#ifdef NDEBUG
#  if __has_builtin(__builtin_assume)
#    define NEPTOOLS_ASSERT(expr) __builtin_assume(expr)
#    define NEPTOOLS_ASSERT_MSG(expr, msg) __builtin_assume(expr)
#  else
#    define NEPTOOLS_ASSERT(expr) ((void)0)
#    define NEPTOOLS_ASSERT_MSG(expr, msg) ((void)0)
#  endif
#  define NEPTOOLS_THROW(...) (throw ::boost::enable_error_info(__VA_ARGS__))

#  if defined(__GNUC__) || defined(__clang__)
#    define NEPTOOLS_UNREACHABLE(x) __builtin_unreachable()
#  else
#    define NEPTOOLS_UNREACHABLE(x) abort()
#  endif

#else

#  include "file.hpp"

#  define NEPTOOLS_ASSERT(expr) \
    (BOOST_LIKELY(!!(expr)) ? ((void)0) : NEPTOOLS_ASSERT_FAILED(#expr, nullptr))

#  define NEPTOOLS_ASSERT_MSG(expr, msg) \
    (BOOST_LIKELY(!!(expr)) ? ((void)0) : NEPTOOLS_ASSERT_FAILED(#expr, msg))

#  define NEPTOOLS_ASSERT_FAILED(expr, msg)                      \
    ::Neptools::AssertFailed(expr, msg, NEPTOOLS_FILE, __LINE__, \
                             NEPTOOLS_FUNCTION)

#  define NEPTOOLS_THROW(...)                               \
    (throw ::boost::enable_error_info(__VA_ARGS__) <<       \
     ::boost::throw_file(NEPTOOLS_FILE) <<                  \
     ::boost::throw_line(__LINE__) <<                       \
     ::boost::throw_function(NEPTOOLS_FUNCTION))
#  define NEPTOOLS_UNREACHABLE(x) NEPTOOLS_ASSERT_FAILED("unreachable", x)
#endif

namespace Neptools
{

BOOST_NORETURN void RethrowBoostException();
std::string ExceptionToString();

#ifndef WINDOWS
BOOST_NORETURN
#endif
void AssertFailed(
    const char* expr, const char* msg, const char* file, unsigned line,
    const char* fun);

template <typename Base, typename T, typename Derived, typename... Args>
inline auto Invoke(T Base::*fun, Derived* thiz, Args&&... args)
{ return (*thiz.*fun)(std::forward<Args>(args)...); }

template <typename Fun, typename... Args>
inline auto Invoke(Fun&& f, Args&&... args)
{ return std::forward<Fun>(f)(std::forward<Args>(args)...); }


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
