#include "../format/item.hpp"
#include "../format/cl3.hpp"
#include "../format/primitive_item.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"
#include "../format/stcm/string_data.hpp"
#include "../format/stsc/file.hpp"
#include "../open.hpp"
#include "../txt_serializable.hpp"
#include "../utils.hpp"
#include "version.hpp"

#include <libshit/except.hpp>
#include <libshit/lua/base.hpp>
#include <libshit/options.hpp>
#include <libshit/platform.hpp>

#include <iostream>
#include <fstream>
#include <deque>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#if LIBSHIT_STDLIB_IS_MSVC
#  undef _CRT_NONSTDC_DEPRECATE // fuck off m$
#  define _CRT_NONSTDC_DEPRECATE(x)
#  include <io.h>
#endif

#define LIBSHIT_LOG_NAME "stcm-editor"
#include <libshit/logger_helper.hpp>

using namespace Neptools;
using namespace Libshit;

namespace
{
  struct State
  {
    SmartPtr<Dumpable> dump;
    Cl3* cl3;
    Stcm::File* stcm;
    TxtSerializable* txt;
  };
}

static State SmartOpen(const boost::filesystem::path& fname)
{
  auto x = OpenFactory::Open(fname);
  return {x, dynamic_cast<Cl3*>(x.get()), dynamic_cast<Stcm::File*>(x.get()),
      dynamic_cast<TxtSerializable*>(x.get())};
}

template <typename T>
static void ShellDump(const T* item, const char* name)
{
  RefCountedPtr<Sink> sink;
  if (name[0] == '-' && name[1] == '\0')
    sink = Sink::ToStdOut();
  else
    sink = Sink::ToFile(name, item->GetSize());
  item->Dump(*sink);
}

template <typename T, typename Fun>
static void ShellInspectGen(const T* item, const char* name, Fun f)
{
  if (name[0] == '-' && name[1] == '\0')
    f(item, std::cout);
  else
    f(item, OpenOut(name));
}

template <typename T>
static void ShellInspect(const T* item, const char* name)
{
  ShellInspectGen(
    item, name, [](auto x, auto&& y) { y << "return " << *x << '\n'; });
}

static void EnsureStcm(State& st)
{
  if (st.stcm) return;
  if (!st.dump) throw InvalidParam{"no file loaded"};
  if (!st.cl3)
    throw InvalidParam{"invalid file loaded: can't find STCM without CL3"};

  st.stcm = &st.cl3->GetStcm();
}

static void EnsureTxt(State& st)
{
  if (st.txt) return;
  EnsureStcm(st);
  if (!st.stcm->GetGbnl())
    LIBSHIT_THROW(DecodeError, "No GBNL found in STCM");
  st.txt = st.stcm;
}

static bool auto_failed = false;
template <typename Pred, typename Fun>
static void RecDo(
  const boost::filesystem::path& path, Pred p, Fun f, bool rec = false)
{
  if (p(path, rec))
  {
    try { f(path); }
    catch (const std::exception& e)
    {
      auto_failed = true;
      ERR << "Failed: " << ExceptionToString() << std::endl;
    }
  }
  else if (boost::filesystem::is_directory(path))
    for (auto& e: boost::filesystem::directory_iterator(path))
      RecDo(e, p, f, true);
  else if (!rec)
    ERR << "Invalid filename: " << path << std::endl;
}

namespace
{
  enum class Mode
  {
#define MODE_PARS_PRE(X)                                                        \
    X(AUTO_STRTOOL,   "auto-strtool",   "import/export .cl3/.gbin/.gstr texts") \
    X(EXPORT_STRTOOL, "export-strtool", "export .cl3/.gbin/.gstr to .txt")      \
    X(IMPORT_STRTOOL, "import-strtool", "import .cl3/.gbin/.gstr from .txt")    \
    X(AUTO_CL3,       "auto-cl3",       "unpack/pack .cl3 files")               \
    X(UNPACK_CL3,     "unpack-cl3",     "unpack .cl3 files")                    \
    X(PACK_CL3,       "pack-cl3",       "pack .cl3 files")
#define MODE_PARS_LUA(X)                                                        \
    X(AUTO_LUA,       "auto-lua",       "import/export stcms")                  \
    X(EXPORT_LUA,     "export-lua",     "export stcms")                         \
    X(IMPORT_LUA,     "import-lua",     "import lua")
#define MODE_PARS_POST(X)                                                       \
    X(MANUAL,         "manual",         "manual processing (set automatically)")
#if LIBSHIT_WITH_LUA
#   define MODE_PARS(X) MODE_PARS_PRE(X) MODE_PARS_LUA(X) MODE_PARS_POST(X)
#else
#   define MODE_PARS(X) MODE_PARS_PRE(X) MODE_PARS_POST(X)
#endif
#define GEN_ENUM(name, shit1, shit2) name,
    MODE_PARS(GEN_ENUM)
#undef GEN_ENUM
  } mode = Mode::AUTO_STRTOOL;
}

static auto BaseDoAutoFun(const boost::filesystem::path& p, const char* ext)
{
  boost::filesystem::path cl3, txt;
  bool import;
  if (boost::ends_with(p.native(), ext))
  {
    cl3 = p.native().substr(0, p.native().size()-4);
    txt = p;
    import = true;
    INF << "Importing: " << cl3 << " <- " << txt << std::endl;
  }
  else
  {
    cl3 = txt = p;
    txt += ext;
    import = false;
    INF << "Exporting: " << cl3 << " -> " << txt << std::endl;
  }

  return std::make_tuple(import, cl3, txt);
}

static void DoAutoTxt(const boost::filesystem::path& p)
{
  auto [import, cl3, txt] = BaseDoAutoFun(p, ".txt");
  auto st = SmartOpen(cl3);
  EnsureTxt(st);
  if (import)
  {
    st.txt->ReadTxt(OpenIn(txt));
    if (st.stcm) st.stcm->Fixup();
    st.dump->Fixup();
    st.dump->Dump(cl3);
  }
  else
    st.txt->WriteTxt(OpenOut(txt));
}

#if LIBSHIT_WITH_LUA
static void DoAutoLua(const boost::filesystem::path& p)
{
  auto [import, bin, lua] = BaseDoAutoFun(p, ".lua");
  if (import)
  {
    Lua::State vm;
    lua_getglobal(vm, "debug"); // +1
    lua_getfield(vm, -1, "traceback"); // +2
    if (luaL_loadfile(vm, lua.string().c_str()) || lua_pcall(vm, 0, 1, -2))
    {
      Logger::Log("lua", Logger::ERROR, nullptr, 0, nullptr)
        << lua_tostring(vm, -1) << std::endl;
      return;
    }
    auto dmp = vm.Get<NotNull<SmartPtr<Dumpable>>>(-1);
    // hack? when importing a cl3, and we get a gbnl, put it into the
    // existing cl3
    if (boost::iends_with(bin.native(), ".cl3") &&
        dynamic_cast<Stcm::File*>(dmp.get()))
    {
      auto cl3 = MakeSmart<Cl3>(Source::FromFile(bin));
      auto stcme = cl3->entries.find("main.DAT", std::less<>{});
      if (stcme == cl3->entries.end())
        LIBSHIT_THROW(DecodeError, "Invalid CL3 file: no main.DAT");
      dmp->Fixup();
      stcme->src = dmp;
      dmp = cl3;
    }
    dmp->Fixup();
    dmp->Dump(bin);
  }
  else
  {
    auto st = SmartOpen(bin);
    EnsureTxt(st);
    OpenOut(lua) << "return " << *(st.stcm ? st.stcm : st.dump.get()) << '\n';
  }
}
#endif

static void DoAutoCl3(const boost::filesystem::path& p)
{
  if (boost::filesystem::is_directory(p))
  {
    boost::filesystem::path cl3_file =
      p.native().substr(0, p.native().size() - 4);
    INF << "Packing " << cl3_file << std::endl;
    Cl3 cl3{Source::FromFile(cl3_file)};
    cl3.UpdateFromDir(p);
    cl3.Fixup();
    cl3.Dump(cl3_file);
  }
  else
  {
    INF << "Extracting " << p << std::endl;
    Cl3 cl3{Source::FromFile(p)};
    auto out = p;
    cl3.ExtractTo(out += ".out");
  }
}

static inline bool is_file(const boost::filesystem::path& pth)
{
  auto stat = boost::filesystem::status(pth);
  return boost::filesystem::is_regular_file(stat) ||
    boost::filesystem::is_symlink(stat);
}

static bool IsBin(const boost::filesystem::path& p, bool = false)
{
  return is_file(p) && (
    boost::iends_with(p.native(), ".cl3") ||
    boost::iends_with(p.native(), ".gbin") ||
    boost::iends_with(p.native(), ".gstr") ||
    boost::iends_with(p.native(), ".bin"));
}

static bool IsTxt(const boost::filesystem::path& p, bool = false)
{
  return is_file(p) && (
    boost::iends_with(p.native(), ".cl3.txt") ||
    boost::iends_with(p.native(), ".gbin.txt") ||
    boost::iends_with(p.native(), ".gstr.txt") ||
    boost::iends_with(p.native(), ".bin.txt"));
}

#if LIBSHIT_WITH_LUA
static bool IsLua(const boost::filesystem::path& p, bool = false)
{
  return is_file(p) && (
    boost::iends_with(p.native(), ".cl3.lua") ||
    boost::iends_with(p.native(), ".gbin.lua") ||
    boost::iends_with(p.native(), ".gstr.lua") ||
    boost::iends_with(p.native(), ".bin.lua"));
}
#endif

static bool IsCl3(const boost::filesystem::path& p, bool = false)
{
  return is_file(p) && boost::iends_with(p.native(), ".cl3");
}

static bool IsCl3Dir(const boost::filesystem::path& p, bool = false)
{
  return boost::filesystem::is_directory(p) &&
    boost::iends_with(p.native(), ".cl3.out");
}

static void DoAuto(const boost::filesystem::path& path)
{
  bool (*pred)(const boost::filesystem::path&, bool);
  void (*fun)(const boost::filesystem::path& p);

  switch (mode)
  {
  case Mode::AUTO_STRTOOL:
    pred = [](auto& p, bool rec)
    {
      if (rec)
        return (IsTxt(p) && boost::filesystem::exists(
                  p.native().substr(0, p.native().size()-4))) ||
          (IsBin(p) && !boost::filesystem::exists(
             boost::filesystem::path(p)+=".txt"));
      else
        return IsBin(p) || IsTxt(p);
    };
    fun = DoAutoTxt;
    break;

  case Mode::EXPORT_STRTOOL:
    pred = IsBin;
    fun = DoAutoTxt;
    break;
  case Mode::IMPORT_STRTOOL:
    pred = IsTxt;
    fun = DoAutoTxt;
    break;

  case Mode::AUTO_CL3:
    pred = [](auto& p, bool rec)
    {
      if (rec)
        return IsCl3Dir(p) || (IsCl3(p) && !boost::filesystem::exists(
                                 boost::filesystem::path(p)+=".out"));
      else
        return IsCl3(p) || IsCl3Dir(p);
    };
    fun = DoAutoCl3;
    break;

  case Mode::UNPACK_CL3:
    pred = IsCl3;
    fun = DoAutoCl3;
    break;
  case Mode::PACK_CL3:
    pred = IsCl3Dir;
    fun = DoAutoCl3;
    break;

#if LIBSHIT_WITH_LUA
  case Mode::AUTO_LUA:
    pred = [](auto& p, bool rec)
    {
      if (rec)
        return (IsLua(p) && boost::filesystem::exists(
                  p.native().substr(0, p.native().size()-4))) ||
          (IsBin(p) && !boost::filesystem::exists(
            boost::filesystem::path(p)+=".lua"));
      else
        return IsBin(p) || IsLua(p);
    };
    fun = DoAutoLua;
    break;

  case Mode::EXPORT_LUA:
    pred = IsBin;
    fun = DoAutoLua;
    break;

  case Mode::IMPORT_LUA:
    pred = IsLua;
    fun = DoAutoLua;
    break;
#endif

  case Mode::MANUAL:
    throw InvalidParam{"Can't use auto files in manual mode"};
  }
  RecDo(path, pred, fun);
}

int main(int argc, char** argv)
{
  State st;
  auto& parser = OptionParser::GetGlobal();
  OptionGroup hgrp{parser, "High-level options"};
  OptionGroup lgrp{parser, "Low-level options", "See README for details"};

  Option mode_opt{
    hgrp, "mode", 'm', 1, "OPTION",
#define GEN_HELP(_, key, help) "\t\t" key ": " help "\n"
    "Set operating mode:\n" MODE_PARS(GEN_HELP),
#undef GEN_HELP
    [](auto&& args)
    {
      if (false); // NOLINT
#define GEN_IFS(c, str, _) else if (strcmp(args.front(), str) == 0) mode = Mode::c;
      MODE_PARS(GEN_IFS)
#undef GEN_IFS
      else throw InvalidParam{"invalid argument"};
    }};

  Option open_opt{
    lgrp, "open", 1, "FILE", "Opens FILE as cl3 or stcm file",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      st = SmartOpen(args.front());
    }};
  Option save_opt{
    lgrp, "save", 1, "FILE|-", "Saves the loaded file to FILE or stdout",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.dump) throw InvalidParam{"no file loaded"};
      st.dump->Fixup();
      ShellDump(st.dump.get(), args.front());
    }};
  Option create_cl3_opt{
    lgrp, "create-cl3", 0, nullptr, "Creates an empty cl3 file",
    [&](auto&&)
    {
      mode = Mode::MANUAL;
      SmartPtr<Cl3> c = MakeSmart<Cl3>();
      st = {c, c.get(), nullptr, nullptr};
    }};
  Option list_files_opt{
    lgrp, "list-files", 0, nullptr, "Lists the contents of the cl3 archive",
    [&](auto&&)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      size_t i = 0;
      for (const auto& e : st.cl3->entries)
      {
        std::cout << i++ << '\t' << e.name << '\t' << e.src->GetSize()
                  << "\tlinks:";
        for (const auto& l : e.links)
          std::cout << ' ' << st.cl3->IndexOf(l);
        std::cout << std::endl;
      }
    }};
  Option extract_file_opt{
    lgrp, "extract-file", 2, "NAME OUT_FILE|-",
    "Extract NAME from cl3 archive to OUT_FILE or stdout",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      auto& entries = st.cl3->entries;
      auto e = entries.find(args[0]);

      if (e == entries.end())
        throw InvalidParam{"specified file not found"};
      else
        ShellDump(e->src.get(), args[1]);
    }};
  Option extract_files_opt{
    lgrp, "extract-files", 1, "DIR", "Extract the cl3 archive to DIR",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      st.cl3->ExtractTo(args.front());
    }};
  Option replace_file_opt{
    lgrp, "replace-file", 2, "NAME IN_FILE",
    "Adds or replaces NAME in cl3 archive with IN_FILE",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};

      auto& e = st.cl3->GetOrCreateFile(args[0]);
      e.src = MakeSmart<DumpableSource>(Source::FromFile(args[1]));
    }};
  Option remove_file_opt{
    lgrp, "remove-file", 1, "NAME", "Removes NAME from cl3 archive",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      auto& entries = st.cl3->entries;
      auto e = entries.find(args.front());
      if (e == entries.end())
        throw InvalidParam{"specified file not found"};
      else
        entries.erase(e);
    }};
  Option set_link_opt{
    lgrp, "set-link", 3, "NAME ID DEST", "Sets link at NAME, ID to DEST",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      auto& entries = st.cl3->entries;
      auto e = entries.find(args[0]);
      auto i = std::stoul(args[1]);
      auto e2 = entries.find(args[2]);
      if (e == entries.end() || e2 == entries.end())
        throw InvalidParam{"specified file not found"};

      if (i < e->links.size())
        e->links[i] = &entries[entries.index_of(e2)];
      else if (i == e->links.size())
        e->links.push_back(&entries[entries.index_of(e2)]);
      else
        throw InvalidParam{"invalid link id"};
    }};
  Option remove_link_opt{
    lgrp, "remove-link", 2, "NAME ID", "Remove link ID from NAME",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.cl3) throw InvalidParam{"no cl3 loaded"};
      auto& entries = st.cl3->entries;
      auto e = entries.find(args[0]);
      auto i = std::stoul(args[1]);
      if (e == entries.end())
        throw InvalidParam{"specified file not found"};

      if (i < e->links.size())
        e->links.erase(e->links.begin() + i);
      else
        throw InvalidParam{"invalid link id"};
    }};
  Option inspect_opt{
    lgrp, "inspect", 1, "OUT|-",
    "Inspects currently loaded file into OUT or stdout",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      if (!st.dump) throw InvalidParam{"No file loaded"};
      ShellInspect(st.dump.get(), args.front());
    }};
  Option inspect_stcm_opt{
    lgrp, "inspect-stcm", 1, "OUT|-",
    "Inspects only the stcm portion of the currently loaded file into OUT or stdout",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      EnsureStcm(st);
      ShellInspect(st.stcm, args.front());
    }};
  Option parse_stcmp_opt{
    lgrp, "parse-stcm", 0, nullptr,
    "Parse STCM-inside-CL3 (usually done automatically)",
    [&](auto&&)
    {
      mode = Mode::MANUAL;
      EnsureStcm(st);
    }};

  Option export_txt_opt{
    lgrp, "export-txt", 1, "OUT_FILE|-", "Export text to OUT_FILE or stdout",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      EnsureTxt(st);
      ShellInspectGen(st.txt, args.front(),
                      [](auto& x, auto&& y) { x->WriteTxt(y); });
    }};
  Option import_txt_opt{
    lgrp, "import-txt", 1, "IN_FILE|-", "Read text from IN_FILE or stdin",
    [&](auto&& args)
    {
      mode = Mode::MANUAL;
      EnsureTxt(st);
      auto fname = args.front();
      if (fname[0] == '-' && fname[1] == '\0')
        st.txt->ReadTxt(std::cin);
      else
        st.txt->ReadTxt(OpenIn(fname));
      if (st.stcm) st.stcm->Fixup();
    }};

#if LIBSHIT_WITH_LUA
  Option lua{
    lgrp, "lua", 'i', 0, nullptr, "Interactive lua prompt",
    [&](auto&&)
    {
      Lua::State vm;
      std::string str;

      // use print (I'm lazy to write my own)
      lua_getglobal(vm, "print"); // 1
      lua_getglobal(vm, "debug"); // 2
      lua_getfield(vm, 2, "traceback"); // 3
      lua_remove(vm, 2); // 2 = traceback

      auto prompt = isatty(0);

      while ((prompt && std::cout << "> ", std::getline(std::cin, str)))
      {
        // if input starts with "> " it's a copy-pasted prompt, remove
        if (boost::algorithm::starts_with(str, "> "))
          str.erase(0, 2);

        lua_pushvalue(vm, 1); // 3 // push print
        if ((luaL_loadstring(vm, ("return "+str).c_str()) &&
             (lua_pop(vm, 1), luaL_loadstring(vm, str.c_str()))) || // 4
            lua_pcall(vm, 0, LUA_MULTRET, 2)) // 3+?
        {
          Logger::Log("lua", Logger::ERROR, nullptr, 0, nullptr)
            << lua_tostring(vm, -1) << std::endl;
          lua_pop(vm, 2);
        }
        else
        {
          auto top = lua_gettop(vm);
          if (top > 3)
            lua_call(vm, top-3, 0);
          else
            lua_pop(vm, 1);
        }
      }
    }};
    Option lua_script{
      lgrp, "lua-script", 'L', 1, "FILE", "Run lua script",
      [&](auto&& args)
      {
        Lua::State vm;
        lua_getglobal(vm, "debug"); // +1
        lua_getfield(vm, -1, "traceback"); // +2
        if (luaL_loadfile(vm, args[0]) || lua_pcall(vm, 0, 0, -2))
          Logger::Log("lua", Logger::ERROR, nullptr, 0, nullptr)
            << lua_tostring(vm, -1) << std::endl;
      }};
#endif

    boost::filesystem::path self{argv[0]};
    if (boost::iequals(self.filename().string(), "cl3-tool")
#if LIBSHIT_OS_IS_WINDOWS
        || boost::iequals(self.filename().string(), "cl3-tool.exe")
#endif
        )
      mode = Mode::AUTO_CL3;

    parser.SetVersion("NepTools stcm-editor v" NEPTOOLS_VERSION);
    parser.SetUsage("[--options] [<file/directory>...]");
    parser.SetShowHelpOnNoOptions();
    parser.SetNonArgHandler(FUNC<DoAuto>);

    try { parser.Run(argc, argv); }
    catch (const Exit& e) { return !e.success; }
    catch (...)
    {
      ERR << "Fatal error, aborting\n" << ExceptionToString() << std::endl;
      return 2;
    }
    return auto_failed;
}
