#include "except.hpp"
#include <boost/core/demangle.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <iostream>

#ifdef WINDOWS
extern "C" void _assert(const char* msg, const char* file, unsigned line);
#endif

namespace Neptools
{

void RethrowBoostException()
{
    try { throw; }
#define RETHROW(ex) catch (const ex& e) { \
        throw boost::enable_error_info(e) << RethrownType{typeid(e)}; }
    RETHROW(std::range_error)
    RETHROW(std::overflow_error)
    RETHROW(std::underflow_error)
    //RETHROW(std::regex_error)
    RETHROW(std::system_error)
    RETHROW(std::runtime_error)
}

void PrintException(std::ostream& os)
{
    try { throw; }
    catch (const boost::exception& e)
    {
        auto se = dynamic_cast<const std::exception*>(&e);
        os << (se ? se->what() : "???")<< "\n\nDetails: "
           << boost::diagnostic_information(e) << std::endl;
    }
    catch (const std::exception& e)
    {
        os << e.what() << std::endl;
    }
    catch (...)
    {
        os << "Unknown exception (run while you can)" << std::endl;
    }
}

void AssertFailed(
    const char* expr, const char* msg, const char* file, unsigned line,
    const char* fun)
{
#ifdef WINDOWS
    std::string fake_expr = expr;
    fake_expr += "\nFunction: ";
    fake_expr += fun;
    if (msg)
    {
        fake_expr += "\nMessage: ";
        fake_expr += msg;
    }
    _assert(fake_expr.c_str(), file, line);
#else
    std::cerr << "Assertion failed!\n" << file << ':' << line
              << ": in function " << fun << "\nExpression: " << expr << '\n';
    if (msg)
        std::cerr << "Message: " << msg << '\n';
    abort();
#endif
}

}

namespace std
{

string to_string(const type_index& type)
{
    return boost::core::demangle(type.name());
}

}
