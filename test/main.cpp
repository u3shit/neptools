#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include <boost/exception/diagnostic_information.hpp>

static std::string info(const boost::exception& e)
{ return boost::diagnostic_information(e); }

int main(int argc, const char** argv)
{
    Catch::ExceptionTranslatorRegistrar x{info};

    return Catch::Session().run( argc, argv );
}
