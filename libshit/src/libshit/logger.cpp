#ifdef WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include <io.h>
#endif

#include "logger.hpp"
#include "options.hpp"
#include "lua/function_call.hpp"

#ifndef LIBSHIT_WITHOUT_LUA
#  include "libshit/logger.lua.h"
#endif

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <map>

#include <boost/tokenizer.hpp>

#ifdef _MSC_VER
#  define strncasecmp _strnicmp
#  define strcasecmp _stricmp
#endif

namespace Libshit::Logger
{

  OptionGroup& GetOptionGroup()
  {
    static OptionGroup grp{OptionParser::GetGlobal(), "Logging options"};
    return grp;
  }

  int global_level = -1;
  bool show_fun = false;

  static std::map<std::string, int, std::less<>> level_map;

  static Option show_fun_opt{
    GetOptionGroup(), "show-functions", 0, nullptr,
#ifdef NDEBUG
    "Ignored for compatibility with debug builds",
#else
    "Show function signatures in log",
#endif
    [](auto&&) { show_fun = true; }};

  static int ParseLevel(const char* str)
  {
    if (strcasecmp(str, "none") == 0)
      return NONE;
    if (strncasecmp(str, "err", 3) == 0)
      return ERROR;
    else if (strncasecmp(str, "warn", 4) == 0)
      return WARNING;
    else if (strncasecmp(str, "info", 4) == 0)
      return INFO;
    else
    {
      char* end;
      auto l = std::strtol(str, &end, 10);
      if (*end)
      {
        std::stringstream ss;
        ss << "Invalid log level " << str;
        throw InvalidParam{ss.str()};
      }
      return l;
    }
  }

  static Option debug_level_opt{
    GetOptionGroup(), "log-level", 'l', 1,
    "[MODULE=LEVEL,[...]][DEFAULT_LEVEL]",
    "Sets logging level for the specified modules, or the global default\n\t"
    "Valid levels: none, err, warn, info"
#ifndef NDEBUG
    ", 0..4 (debug levels)"
#endif
    "\n\tDefault level: info",
    [](auto&& args)
    {
      boost::char_separator<char> sep{","};
      auto arg = args.front();
      boost::tokenizer<boost::char_separator<char>, const char*>
        tokens{arg, arg+strlen(arg), sep};
      for (const auto& tok : tokens)
      {
        auto p = tok.find_first_of('=');
        if (p == std::string::npos)
          global_level = ParseLevel(tok.c_str());
        else
        {
          auto lvl = ParseLevel(tok.c_str() + p + 1);
          level_map[tok.substr(0, p)] = lvl;
        }
      }
    }};

  static auto& os = std::clog;

  namespace
  {
    struct LogBuffer final : public std::streambuf
    {
      LogBuffer()
      {
        // otherwise clog is actually unbuffered...
        static char buf[4096];
        // buf can be nullptr on linux (and posix?), but crashes on windows...
        setvbuf(stderr, buf, _IOFBF, 4096);

#ifdef WINDOWS
        colors = _isatty(2);
#else
        const char* x;
        if (isatty(2) && (x = getenv("TERM")) ? strcmp(x, "dummy") != 0 : false)
          colors = true;
#endif
      }

      std::streamsize xsputn(const char* msg, std::streamsize n) override
      {
        auto old_n = n;
        while (n)
        {
          if (line_begin)
            WriteBegin();

          auto end = std::find(msg, msg+n, '\n');
          os.write(msg, end-msg);
          if (end < msg+n)
          {
            // we had a newline
            line_begin = true;
            ++end;
            WriteEnd();
          }

          n -= end-msg;
          msg = end;
        }
        return old_n;
      }

      int_type overflow(int_type ch) override
      {
        if (ch != traits_type::eof())
        {
          char c = ch;
          LogBuffer::xsputn(&c, 1);
        }
        return 0;
      }

      int sync() override
      {
        os.flush();
        return 0;
      }


      void WriteBegin()
      {
#ifdef WINDOWS
        HANDLE h;
        int color;
#endif
        if (colors)
        {
#ifdef WINDOWS
          os.flush();
          h = GetStdHandle(STD_ERROR_HANDLE);
          switch (level)
          {
          case ERROR:   color = FOREGROUND_RED;                    break;
          case WARNING: color = FOREGROUND_RED | FOREGROUND_GREEN; break;
          case INFO:    color = FOREGROUND_GREEN;                  break;
          }
          SetConsoleTextAttribute(h, FOREGROUND_INTENSITY | color);
#else
          switch (level)
          {
          case ERROR:   os << "\033[1;31m"; break;
          case WARNING: os << "\033[1;33m"; break;
          case INFO:    os << "\033[1;32m"; break;
          default:      os << "\033[1m";    break;
          }
#endif
        }
        switch (level)
        {
        case ERROR:   os << "ERROR"; break;
        case WARNING: os << "WARN "; break;
        case INFO:    os << "info "; break;
        default:      os << "dbg" << std::setw(2) << level; break;
        }

        max_name = std::max(max_name, std::strlen(name));
        os << '[' << std::setw(max_name) << name << ']';
        if (colors)
        {
#ifdef WINDOWS
          os.flush();
          SetConsoleTextAttribute(h, color);
#else
          os << "\033[22m";
#endif
        }

#ifndef NDEBUG
        if (file)
        {
          max_file = std::max(max_file, std::strlen(file));
          os << ' ' << std::setw(max_file) << file << ':'
             << std::setw(3) << line;
        }
        if (show_fun && fun)
        {
          max_fun = std::max(max_fun, std::strlen(fun));
          os << ' ' << std::setw(max_fun) << fun;
        }
#endif
        os << ": ";

        line_begin = false;
      }

      void WriteEnd()
      {
        if (colors)
        {
#ifdef WINDOWS
          os.flush();
          SetConsoleTextAttribute(
            GetStdHandle(STD_ERROR_HANDLE),
            FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
          os << "\033[0m";
#endif
        }
        os << '\n';
      }

      bool colors = false;
      bool line_begin = true;
      const char* name;
      size_t max_name = 8;
      int level;
#ifndef NDEBUG
      const char* file;
      unsigned line;
      const char* fun;
      size_t max_file = 20, max_fun = 20;
#endif
    };
  }

  static LogBuffer filter;
  static std::ostream log_os{&filter};

  bool CheckLog(const char* name, int level) noexcept
  {
    auto it = level_map.find(name);
    if (it != level_map.end()) return it->second >= level;
    else return global_level >= level;
  }

  std::ostream& Log(
    const char* name, int level, const char* file, unsigned line,
    const char* fun)
  {
    filter.name = name;
    filter.level = level;
#ifndef NDEBUG
    filter.file = file;
    filter.line = line;
    filter.fun = fun;
#else
    // silence argument unused warnings
    (void) file;
    (void) line;
    (void) fun;
#endif
    return log_os;
  }

#ifndef LIBSHIT_WITHOUT_LUA
  static void LuaLog(
    Lua::StateRef vm, const char* name, int level, Lua::Skip msg)
  {
    (void) msg;
    const char* file = nullptr;
    unsigned line = 0;
    const char* fun = nullptr;

#  ifndef NDEBUG
    lua_Debug dbg;
    if (lua_getstack(vm, 1, &dbg) && lua_getinfo(vm, "Sln", &dbg))
    {
      file = dbg.short_src;
      line = dbg.currentline;
      fun = dbg.name;
    }
#  endif
    auto& os = Log(name, level, file, line, fun);

    size_t len;
    auto str = luaL_tolstring(vm, 3, &len); // +1
    os.write(str, len);
    lua_pop(vm, 1); // 0

    os << std::flush;
  }

  static Lua::State::Register reg{[](Lua::StateRef vm)
    {
      lua_createtable(vm, 0, 2); // +1

      vm.PushFunction<&CheckLog>(); // +2
      lua_setfield(vm, -2, "check_log"); // +1

      vm.PushFunction<&LuaLog>(); // +2
      lua_setfield(vm, -2, "raw_log"); // +1

      vm.Push(Level::ERROR); // +2
      lua_setfield(vm, -2, "ERROR"); // +1

      vm.Push(Level::WARNING); // +2
      lua_setfield(vm, -2, "WARNING"); // +1

      vm.Push(Level::INFO); // +2
      lua_setfield(vm, -2, "INFO"); // +1

      lua_pushglobaltable(vm); // +2
      vm.SetRecTable("libshit.log", -2); // +1
      lua_pop(vm, 1); // 0

      LIBSHIT_LUA_RUNBC(vm, logger, 0);
    }};

#endif
}
