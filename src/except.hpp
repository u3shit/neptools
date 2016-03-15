#ifndef UUID_4999021A_F41A_400B_A951_CDE022AF7331
#define UUID_4999021A_F41A_400B_A951_CDE022AF7331
#pragma once

#include <stdexcept>
#include <typeindex>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/info.hpp>

#ifdef NDEBUG
#define NEPTOOLS_THROW(x) ::boost::throw_exception(x)

#if defined(__GNUC__) || defined(__clang__)
#define NEPTOOLS_UNREACHABLE(x) __builtin_unreachable()
#else
#define NEPTOOLS_UNREACHABLE(x) abort()
#endif

#else

#define NEPTOOLS_THROW BOOST_THROW_EXCEPTION
#define NEPTOOLS_UNREACHABLE(x) BOOST_ASSERT_MSG(false, x);
#endif

#if defined(__GNUC__) || defined(__clang__)
#define NEPTOOLS_NORETURN __attribute__((noreturn))
#else
#define NEPTOOLS_NORETURN
#endif

namespace Neptools
{

NEPTOOLS_NORETURN void RethrowBoostException();
void PrintException(std::ostream& os);

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

struct DecodeError : std::runtime_error, virtual boost::exception
{
    using std::runtime_error::runtime_error;
};

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
