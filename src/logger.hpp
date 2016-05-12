#ifndef UUID_AA3FD944_B99F_4F56_B342_FFE9A6ACD9FE
#define UUID_AA3FD944_B99F_4F56_B342_FFE9A6ACD9FE
#pragma once

#include "file.hpp"
#include <iosfwd>

namespace Neptools
{

class OptionGroup;
namespace Logger
{

#undef ERROR // fuck you windows.h

enum Level
{
    NONE = -4,
    ERROR = -3,
    WARNING = -2,
    INFO = -1,
};

OptionGroup& GetOptionGroup();
extern int global_level;

bool CheckLog(const char* name, int level);
std::ostream& Log(
    const char* name, int level, const char* file, unsigned line,
    const char* fun);

#ifdef NDEBUG
#  define NEPTOOLS_LOG_ARGS nullptr, 0, nullptr
#else
#  define NEPTOOLS_LOG_ARGS NEPTOOLS_FILE, __LINE__, NEPTOOLS_FUNCTION
#endif

#define NEPTOOLS_LOG(name, level)                           \
    ::Neptools::Logger::CheckLog(name, level) &&            \
    ::Neptools::Logger::Log(name, level, NEPTOOLS_LOG_ARGS)

#define NEPTOOLS_ERR(name)  NEPTOOLS_LOG(name, ::Neptools::Logger::ERROR)
#define NEPTOOLS_WARN(name) NEPTOOLS_LOG(name, ::Neptools::Logger::WARNING)
#define NEPTOOLS_INFO(name) NEPTOOLS_LOG(name, ::Neptools::Logger::INFO)

#ifndef NDEBUG
#  define NEPTOOLS_DBG(name, level)                                             \
    ([]{static_assert(0 <= (level) && (level) < 5, "invalid debug level");},1) && \
    NEPTOOLS_LOG(name, level)
#else
#  define NEPTOOLS_DBG(name, level) \
    while (false) *static_cast<std::ostream*>(nullptr)
#endif

}
}

#endif
