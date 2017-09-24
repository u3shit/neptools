#include "../injected/cpk.hpp"
#include "../injected/hook.hpp"
#include "../pattern_parse.hpp"
#include "version.hpp"

#include <libshit/options.hpp>

// factory
#include "../format/cl3.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"
#include "../format/primitive_item.hpp"
#include "../format/stcm/string_data.hpp"

#define LIBSHIT_LOG_NAME "server"
#include <libshit/logger_helper.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <io.h>

extern "C"
HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD version, REFIID iid,
                                  LPVOID* out, void* unk) noexcept
{
  {
    auto sys_len = GetSystemDirectory(nullptr, 0);
    if (sys_len == 0) goto err;
    auto len = sys_len + strlen("/dinput8.dll");
    std::unique_ptr<wchar_t[]> buf{new wchar_t[len]};
    if (GetSystemDirectoryW(buf.get(), sys_len) != sys_len-1) goto err;
    wcscat(buf.get(), L"\\dinput8.dll");

    auto dll = LoadLibraryW(buf.get());
    if (dll == nullptr) goto err;
    auto proc = reinterpret_cast<decltype(&DirectInput8Create)>(
      GetProcAddress(dll, "DirectInput8Create"));
    if (proc == nullptr) goto err;
    return proc(inst, version, iid, out, unk);
  }

err:
  MessageBoxA(nullptr, "DirectInput8Create loading failed", "Neptools",
              MB_OK | MB_ICONERROR);
  abort();
}

using namespace Neptools;
using namespace Libshit;

// bring in required shit for OpenFactory/DataFactory
// do not actually call this, as it would crash
static void Dependencies()
{
  Source* src = nullptr;
  Cl3 cl3{*src};
  Stcm::File stcm{*src};
  stcm.Create<Stcm::GbnlItem>(*src);
  stcm.Create<Int32Item>(*src);
  stcm.Create<Stcm::StringDataItem>("");
}

static std::string UnfuckString(wchar_t* str)
{
  auto req = WideCharToMultiByte(
    CP_ACP, 0, str, -1, nullptr, 0, nullptr, nullptr);
  if (req == 0)
    LIBSHIT_THROW(std::runtime_error, "Invalid command line parameters");
  std::string ret;
  ret.resize(req-1);
  auto r2 = WideCharToMultiByte(
    CP_ACP, 0, str, -1, &ret[0], req, nullptr, nullptr);
  if (r2 != req)
    LIBSHIT_THROW(std::runtime_error, "Invalid command line parameters");
  return ret;
}

namespace
{
  class MsgboxStringStream : public std::stringstream
  {
  public:
    using std::stringstream::stringstream;
    ~MsgboxStringStream()
    {
      if (!str().empty())
        MessageBoxA(nullptr, str().c_str(), "Neptools", MB_OK | MB_ICONERROR);
    }
  };
}

static OptionGroup server_grp{OptionParser::GetGlobal(), "Server options"};
static bool disable = false;
static Option disable_opt{
  server_grp, "disable", 0, nullptr, "Disable function hooking",
  [](auto&&) { disable = true; }};

static Option console_opt{
  Logger::GetOptionGroup(), "console", 'c', 0, nullptr,
  "Log to a console window",
  [](auto&&)
  {
    AllocConsole();
    SetConsoleTitleA("NepTools Console");
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    INF << "Console init" << std::endl;
  }};

static Option file_opt{
  Logger::GetOptionGroup(), "log-to-file", 'f', 1, "FILENAME",
  "Redirect logging messages to file",
  [](auto&& args)
  {
    freopen(args.front(), "w", stdout);
    _dup2(_fileno(stdout), _fileno(stderr));
    INF << "Logging to file " << args.front() << std::endl;
  }};

static void PrintRecord(const EXCEPTION_RECORD* er, int lvl = 0)
{
  std::string pref(2*lvl+1, ' ');
#define X(fld) ERR << pref << #fld ": " << er->fld << '\n'
  ERR << std::hex;
  X(ExceptionCode); X(ExceptionFlags); X(ExceptionAddress);
  ERR << std::dec;
  X(NumberParameters);
#undef X
  for (size_t i = 0; i < er->NumberParameters; ++i)
    ERR << pref << "ExceptionInformation[" << std::dec << i << "]: "
        << std::hex << er->ExceptionInformation[i] << '\n';
  if (er->ExceptionRecord) PrintRecord(er->ExceptionRecord, lvl+1);
}

static int Filter(unsigned code, EXCEPTION_POINTERS* ep)
{
  ERR << "Seh error 0x" << std::hex << code << '\n';
  PrintRecord(ep->ExceptionRecord);
  auto ctx = ep->ContextRecord;
#define X(fld) ERR << "+" #fld ": " << ctx->fld << '\n';
  ERR << std::hex;
  if (ctx->ContextFlags & CONTEXT_SEGMENTS)
  { X(SegGs); X(SegFs); X(SegEs); X(SegDs); }
  if (ctx->ContextFlags & CONTEXT_INTEGER)
  { X(Eax); X(Ebx); X(Ecx); X(Edx); X(Esi); X(Edi); }
  if (ctx->ContextFlags & CONTEXT_CONTROL)
  { X(Ebp); X(Eip); X(Esp); X(SegCs); X(SegSs); X(EFlags); }
  ERR << std::dec << std::flush;

  return EXCEPTION_CONTINUE_SEARCH;
}

using WinMainPtr = int (CALLBACK*)(HINSTANCE, HINSTANCE, wchar_t*, int);
static WinMainPtr orig_main;

static void* dll_base;

static int CALLBACK NewWinMain2(
  HINSTANCE inst, HINSTANCE prev, wchar_t* cmdline, int show_cmd)
{
  try
  {
    int argc;
    auto argw = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> argv;
    std::unique_ptr<const char*[]> cargv(new const char*[argc+1]);
    argv.reserve(argc);

    for (int i = 0; i < argc; ++i)
    {
      argv.push_back(UnfuckString(argw[i]));
      cargv[i] = argv[i].c_str();
    }
    LocalFree(argw);
    cargv[argc] = nullptr;

    MsgboxStringStream ss;
    auto& pars = OptionParser::GetGlobal();
    pars.SetVersion("NepTools server v" NEPTOOLS_VERSION);
    pars.SetUsage("[--options]");
    pars.FailOnNoArg();
    pars.SetOstream(ss);

    try { pars.Run(argc, cargv.get()); }
    catch (const Exit& e) { return !e.success; }

    if (!disable)
    {
      DBG(1) << "Image base = " << static_cast<void*>(image_base)
             << ", dll base = " << dll_base << std::endl;
      CpkHandler::Init();
      DBG(0) << "Hook done" << std::endl;
    }
  }
  catch (const std::exception& e)
  {
    ERR << "Exception during NewWinMain: " << ExceptionToString()
        << std::endl;
    MessageBoxA(nullptr, e.what(), "WinMain", MB_OK | MB_ICONERROR);
    return -1;
  }


  DBG(0) << "Starting main" << std::endl;
  return orig_main(inst, prev, cmdline, show_cmd);
}

static int CALLBACK NewWinMain(
  HINSTANCE inst, HINSTANCE prev, wchar_t* cmdline, int show_cmd)
{
  __try { return NewWinMain2(inst, prev, cmdline, show_cmd); }
  __except (Filter(GetExceptionCode(), GetExceptionInformation()))
  { abort(); }
}

// msvc 2013 crt offset between entry point and call to WinMain+1
static constexpr size_t MAIN_CALL_OFFSET = -201+1;

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID)
{
  if (reason != DLL_PROCESS_ATTACH) return true;
  if (inst == nullptr) Dependencies();
  dll_base = inst;

  DisableThreadLibraryCalls(inst);

  image_base = reinterpret_cast<Byte*>(GetModuleHandle(nullptr));
  auto call = GetEntryPoint() + MAIN_CALL_OFFSET;

  Unprotect up{call, 4};
  orig_main = reinterpret_cast<WinMainPtr>(call + 4 + As<size_t>(call));
  As<size_t>(call) = reinterpret_cast<Byte*>(NewWinMain) - call - 4;

  return true;
}
