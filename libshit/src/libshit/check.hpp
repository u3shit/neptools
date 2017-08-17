#pragma once

#include "assert.hpp"
#include "except.hpp"

#ifdef NDEBUG
#define NEPTOOLS_CHECK_ARGS nullptr, 0, nullptr
#else
#define NEPTOOLS_CHECK_ARGS NEPTOOLS_FILE, __LINE__, NEPTOOLS_FUNCTION
#endif

#define NEPTOOLS_CHECK(except_type, x, msg)                     \
    Checker{}.template Check<except_type>(                      \
        [&]() { return (x); }, #x, msg, NEPTOOLS_CHECK_ARGS)

namespace Neptools
{
namespace Check
{

struct No
{
    template <typename ExceptT, typename Fun>
    void Check(Fun f, const char*, const char*, const char*, unsigned,
               const char*) noexcept
    { NEPTOOLS_ASSUME(f()); (void) f; }

    static constexpr bool IS_NOP = true;
    static constexpr bool IS_NOEXCEPT = true;
};

struct DoAssert
{
    template <typename ExceptT, typename Fun>
    void Check(Fun f, const char* expr, const char* msg, const char* file,
               unsigned line, const char* fun)
    {
        if (BOOST_UNLIKELY(!f()))
            AssertFailed(expr, msg, file, line, fun);
    }

    static constexpr bool IS_NOP = false;
    // theoretically Fun can throw, but it probably shouldn't do it. also
    // marking this false could cause havoc as functions using
    // noexcept(Checker::IS_NOEXCEPT)  would have different noexceptness on
    // debug and release
    static constexpr bool IS_NOEXCEPT = true;
};

#ifndef NDEBUG
using Assert = DoAssert;
#else
using Assert = No;
#endif


struct Throw
{
    template <typename ExceptT, typename Fun>
    void Check(Fun f, const char* expr, const char* msg, const char* file,
               unsigned line, const char* fun)
    {
        if (!f())
            throw ::boost::enable_error_info(ExceptT{msg}) <<
#ifndef NDEBUG
                ::boost::throw_file(file) <<
                ::boost::throw_line(line) <<
                ::boost::throw_function(fun) <<
#endif
                FailedExpression{expr};
        ((void) file); ((void) line); ((void) fun);
    }

    static constexpr bool IS_NOP = false;
    static constexpr bool IS_NOEXCEPT = false;
};

}
}
