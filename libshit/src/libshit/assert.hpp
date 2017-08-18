#ifndef UUID_1846E497_4817_47F8_8588_147E6B9962C9
#define UUID_1846E497_4817_47F8_8588_147E6B9962C9
#pragma once

#include <boost/config.hpp>

#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_unreachable) || defined(__GNUC__)
#  define LIBSHIT_BUILTIN_UNREACHABLE() __builtin_unreachable()
#else
#  define LIBSHIT_BUILTIN_UNREACHABLE() abort()
#endif

#if __has_builtin(__builtin_assume)
#  define LIBSHIT_ASSUME(expr) __builtin_assume(!!(expr))
#else
#  define LIBSHIT_ASSUME(expr) ((void) 0)
#endif

#ifdef NDEBUG
#  define LIBSHIT_ASSERT(expr) LIBSHIT_ASSUME(expr)
#  define LIBSHIT_ASSERT_MSG(expr, msg) LIBSHIT_ASSUME(expr)
#  define LIBSHIT_UNREACHABLE(x) LIBSHIT_BUILTIN_UNREACHABLE()

#else

#  include "file.hpp"

#  define LIBSHIT_ASSERT(expr) \
  (BOOST_LIKELY(!!(expr)) ? ((void)0) : LIBSHIT_ASSERT_FAILED(#expr, nullptr))

#  define LIBSHIT_ASSERT_MSG(expr, msg) \
  (BOOST_LIKELY(!!(expr)) ? ((void)0) : LIBSHIT_ASSERT_FAILED(#expr, msg))

#  define LIBSHIT_ASSERT_FAILED(expr, msg)                   \
  ::Libshit::AssertFailed(expr, msg, LIBSHIT_FILE, __LINE__, \
                          LIBSHIT_FUNCTION)

#  define LIBSHIT_UNREACHABLE(x)             \
  do                                         \
  {                                          \
    LIBSHIT_ASSERT_FAILED("unreachable", x); \
    /* click on ignore -> you're toasted */  \
    LIBSHIT_BUILTIN_UNREACHABLE();           \
  } while (false)
#endif

namespace Libshit
{
#ifndef WINDOWS
  BOOST_NORETURN
#endif
  void AssertFailed(
    const char* expr, const char* msg, const char* file, unsigned line,
    const char* fun);

}

#endif
