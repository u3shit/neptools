#include "except.hpp"
#include <boost/core/demangle.hpp>
#include <boost/exception/diagnostic_information.hpp>

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

}

namespace std
{

string to_string(const type_index& type)
{
    return boost::core::demangle(type.name());
}

}
