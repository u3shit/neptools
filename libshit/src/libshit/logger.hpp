#ifndef UUID_AA3FD944_B99F_4F56_B342_FFE9A6ACD9FE
#define UUID_AA3FD944_B99F_4F56_B342_FFE9A6ACD9FE
#pragma once

#include "file.hpp"
#include "lua/type_traits.hpp"

#include <iosfwd>

namespace Libshit { class OptionGroup; }

namespace Libshit::Logger
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
  extern bool show_fun;

  bool CheckLog(const char* name, int level) noexcept;
  std::ostream& Log(
    const char* name, int level, const char* file, unsigned line,
    const char* fun);

#ifdef NDEBUG
#  define LIBSHIT_LOG_ARGS nullptr, 0, nullptr
#else
#  define LIBSHIT_LOG_ARGS LIBSHIT_FILE, __LINE__, LIBSHIT_FUNCTION
#endif

#define LIBSHIT_LOG(name, level)              \
  ::Libshit::Logger::CheckLog(name, level) && \
  ::Libshit::Logger::Log(name, level, LIBSHIT_LOG_ARGS)

#define LIBSHIT_ERR(name)  LIBSHIT_LOG(name, ::Libshit::Logger::ERROR)
#define LIBSHIT_WARN(name) LIBSHIT_LOG(name, ::Libshit::Logger::WARNING)
#define LIBSHIT_INFO(name) LIBSHIT_LOG(name, ::Libshit::Logger::INFO)

#ifndef NDEBUG
#  define LIBSHIT_DBG(name, level)                                              \
  ([]{static_assert(0 <= (level) && (level) < 5, "invalid debug level");},1) && \
  LIBSHIT_LOG(name, level)
#else
#  define LIBSHIT_DBG(name, level) \
  while (false) *static_cast<std::ostream*>(nullptr)
#endif

}

LIBSHIT_ENUM(Libshit::Logger::Level);

#endif
