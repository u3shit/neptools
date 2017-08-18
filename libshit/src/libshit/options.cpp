#include "options.hpp"
#include <iostream>
#include <array>
#include <map>
#include <cstring>
#include <climits>
#include <boost/exception/get_error_info.hpp>

namespace Libshit
{

  OptionGroup::OptionGroup(
    OptionParser& parser, const char* name, const char* help)
    : name{name}, help{help}
  { parser.groups.push_back(this); }

  OptionParser::OptionParser()
    : help_version{*this, "General options"},
      help_option{
        help_version, "help", 'h', 0, nullptr, "Show this help message",
        [this](auto&&) { this->ShowHelp(); throw Exit{true}; }},
      version_option{
        help_version, "version", 0, nullptr, "Show program version",
        [this](auto&&) { *os << version << std::endl; throw Exit{true}; }},
      os{&std::clog}
  {
    version_option.enabled = false;
  }

  void OptionParser::SetVersion(const char* version_str)
  {
    version = version_str;
    version_option.enabled = true;
  }

  void OptionParser::FailOnNoArg()
  {
    no_arg_fun = [](auto) { throw InvalidParam{"Invalid option"}; };
  }

  namespace
  {
    struct OptCmp
    {
      bool operator()(const char* a, const char* b) const
      {
        while (*a && *a == *b) ++a, ++b;

        if (*a == '=' || *b == '=') return false;
        return *reinterpret_cast<const unsigned char*>(a) <
          *reinterpret_cast<const unsigned char*>(b);
      }
    };
  }

  static size_t ParseShort(const std::array<Option*, 256>& short_opts,
                           size_t argc, size_t i, const char** argv)
  {
    auto ptr = argv[i];
    return AddInfo([&]() -> size_t
    {
      while (*++ptr)
      {
        auto opt = short_opts[static_cast<unsigned char>(*ptr)];
        if (!opt) throw InvalidParam{"Unknown option"};

        std::vector<const char*> args;
        if (opt->args_count)
        {
          auto count = opt->args_count;
          args.reserve(count);

          if (ptr[1])
          {
            args.push_back(ptr+1);
            --count;
          }

          ++i;
          for (size_t j = 0; j < count; ++j)
          {
            if (i+j >= argc) throw InvalidParam{"Not enough arguments"};

            args.push_back(argv[i+j]);
          }

          opt->func(std::move(args));
          return count;
        }

        opt->func(std::move(args));
      }
      return 0;
    }, [&](auto& e) { e << ProcessedOption{{'-', *ptr}}; });
  }

  static size_t ParseLong(
    const std::map<const char*, Option*, OptCmp>& long_opts,
    size_t argc, size_t i, const char** argv)
  {
    auto arg = strchr(argv[i]+2, '=');
    auto len = arg ? arg - argv[i] - 2 : strlen(argv[i]+2);

    auto it = long_opts.lower_bound(argv[i]+2);
    if (it == long_opts.end() || strncmp(argv[i]+2, it->first, len))
      throw InvalidParam{"Unknown option"};

    if (std::next(it) != long_opts.end() &&
        it->first[len] != '\0' && // exact matches are non ambiguous
        strncmp(argv[i]+2, std::next(it)->first, len) == 0)
    {
      std::stringstream ss;
      ss << "Ambiguous option (candidates:";
      for (; it != long_opts.end() && strncmp(argv[i]+2, it->first, len) == 0;
           ++it)
        ss << " --" << it->first;
      ss << ")";
      throw InvalidParam{ss.str()};
    }

    auto opt = it->second;

    auto count = opt->args_count;
    std::vector<const char*> args;
    args.reserve(count);

    if (arg)
      if (opt->args_count == 0)
        throw InvalidParam{"Option doesn't take arguments"};
      else
      {
        args.push_back(arg+1);
        --count;
      }

    ++i;
    for (size_t j = 0; j < count; ++j)
    {
      if (i+j >= argc) throw InvalidParam{"Not enough arguments"};
      args.push_back(argv[i+j]);
    }

    opt->func(std::move(args));
    return count;
  }

  void OptionParser::Run_(int& argc, const char** argv)
  {
    argv0 = argv[0];
    int endp = 1;
    if (argc == 1)
    {
      if (no_opts_help)
      {
        ShowHelp();
        throw Exit{true};
      }
      else return;
    }

    std::array<Option*, 256> short_opts{};
    static_assert(CHAR_BIT == 8);
    std::map<const char*, Option*, OptCmp> long_opts;

    for (auto g : groups)
      for (auto o : g->GetOptions())
      {
        if (!o->enabled) continue;
        if (o->short_name)
        {
          if (short_opts[static_cast<unsigned char>(o->short_name)])
            LIBSHIT_THROW(std::logic_error{"Duplicate short option"});
          short_opts[static_cast<unsigned char>(o->short_name)] = o;
        }

        auto x = long_opts.insert(std::make_pair(o->name, o));
        if (!x.second)
          LIBSHIT_THROW(std::logic_error{"Duplicate long option"});
      }

    for (int i = 1; i < argc; ++i)
    {
      // option: "--"something, "-"something
      // non option: something, "-"
      // special: "--"

      // non option
      if (argv[i][0] != '-' || (argv[i][0] == '-' && argv[i][1] == '\0'))
      {
        if (no_arg_fun) no_arg_fun(argv[i]);
        else argv[endp++] = argv[i];
      }
      else
      {
        if (argv[i][1] != '-') // short
          i += ParseShort(short_opts, argc, i, argv);
        else if (argv[i][1] == '-' && argv[i][2] != '\0') // long
          i += AddInfo(std::bind(ParseLong, long_opts, argc, i, argv),
                       [=](auto& e) { e << ProcessedOption{argv[i]}; });
        else // --: end of args
        {
          if (no_arg_fun)
            for (++i; i < argc; ++i)
              no_arg_fun(argv[i]);
          else
            for (++i; i < argc; ++i)
              argv[endp++] = argv[i];
          break;
        }
      }
    }

    argc = endp;
    argv[argc] = nullptr;
  }

  void OptionParser::Run(int& argc, const char** argv)
  {
    try { Run_(argc, argv); }
    catch (const InvalidParam& p)
    {
      *os << *boost::get_error_info<ProcessedOption>(p) << ": "
          << p.what() << std::endl;
      throw Exit{false};
    }
  }

  void OptionParser::ShowHelp()
  {
    if (version) *os << version << "\n\n";
    if (usage) *os << "Usage:\n\t" << argv0 << " " << usage << "\n\n";

    for (auto g : groups)
    {
      if (g != groups[0]) *os << '\n';
      *os << g->GetName() << ":\n";
      if (g->GetHelp()) *os << g->GetHelp() << '\n';

      for (auto o : g->GetOptions())
      {
        if (!o->enabled) continue;
        if (o->short_name) *os << " -" << o->short_name;
        else *os << "   ";

        *os << " --" << o->name;
        if (o->args_help) *os << '=' << o->args_help;
        *os << '\n';
        if (o->help) *os << '\t' << o->help << '\n';
      }
    }
  }

}
