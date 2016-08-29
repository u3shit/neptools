#ifndef UUID_1846E497_4817_47F8_8588_147E6B9962C9
#define UUID_1846E497_4817_47F8_8588_147E6B9962C9
#pragma once

#include <boost/config.hpp>

#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__)
#  define NEPTOOLS_BUILTIN_UNREACHABLE() __builtin_unreachable()
#else
#  define NEPTOOLS_BUILTIN_UNREACHABLE() abort()
#endif

#if __has_builtin(__builtin_assume)
#  define NEPTOOLS_ASSUME(expr) __builtin_assume(!!(expr))
#else
#  define NEPTOOLS_ASSUME(expr) ((void) 0)
#endif

#ifdef NDEBUG
#  define NEPTOOLS_ASSERT(expr) NEPTOOLS_ASSUME(expr)
#  define NEPTOOLS_ASSERT_MSG(expr, msg) NEPTOOLS_ASSUME(expr)
#  define NEPTOOLS_UNREACHABLE(x) NEPTOOLS_BUILTIN_UNREACHABLE()

#else

#  include "file.hpp"

#  define NEPTOOLS_ASSERT(expr) \
    (BOOST_LIKELY(!!(expr)) ? ((void)0) : NEPTOOLS_ASSERT_FAILED(#expr, nullptr))

#  define NEPTOOLS_ASSERT_MSG(expr, msg) \
    (BOOST_LIKELY(!!(expr)) ? ((void)0) : NEPTOOLS_ASSERT_FAILED(#expr, msg))

#  define NEPTOOLS_ASSERT_FAILED(expr, msg)                      \
    ::Neptools::AssertFailed(expr, msg, NEPTOOLS_FILE, __LINE__, \
                             NEPTOOLS_FUNCTION)

#  define NEPTOOLS_UNREACHABLE(x)                   \
    do                                              \
    {                                               \
        NEPTOOLS_ASSERT_FAILED("unreachable", x);   \
        /* click on ignore -> you're toasted */     \
        NEPTOOLS_BUILTIN_UNREACHABLE();             \
    } while (0)
#endif

namespace Neptools
{
#ifndef WINDOWS
BOOST_NORETURN
#endif
void AssertFailed(
    const char* expr, const char* msg, const char* file, unsigned line,
    const char* fun);

}

#endif
