#include "../injected/cpk.hpp"
#include "../injected/hook.hpp"
#include "../pattern_parse.hpp"
#include "../options.hpp"
#include "version.hpp"

#define NEPTOOLS_LOG_NAME "server"
#include "../logger_helper.hpp"

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

namespace
{

static std::string UnfuckString(wchar_t* str)
{
    auto req = WideCharToMultiByte(
        CP_ACP, 0, str, -1, nullptr, 0, nullptr, nullptr);
    if (req == 0)
        NEPTOOLS_THROW(std::runtime_error{"Invalid command line parameters"});
    std::string ret;
    ret.resize(req-1);
    auto r2 = WideCharToMultiByte(
        CP_ACP, 0, str, -1, &ret[0], req, nullptr, nullptr);
    if (r2 != req)
        NEPTOOLS_THROW(std::runtime_error{"Invalid command line parameters"});
    return ret;
}

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

OptionGroup server_grp{OptionParser::GetGlobal(), "Server options"};
bool disable = false;
Option disable_opt{
    server_grp, "disable", 0, nullptr, "Disable function hooking",
    [](auto&&) { disable = true; }};

Option console_opt{
    Logger::GetOptionGroup(), "console", 'c', 0, nullptr,
    "Log to a console window",
    [](auto&&)
    {
        AllocConsole();
        SetConsoleTitleA("NepTools Console");
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        INFO << "Console init" << std::endl;
    }};

Option file_opt{
    Logger::GetOptionGroup(), "log-to-file", 'f', 1, "FILENAME",
    "Redirect logging messages to file",
    [](auto&& args)
    {
        freopen(args.front(), "w", stdout);
        _dup2(_fileno(stdout), _fileno(stderr));
        INFO << "Logging to file " << args.front() << std::endl;
    }};

using WinMainPtr = int (CALLBACK*)(HINSTANCE, HINSTANCE, wchar_t*, int);
WinMainPtr orig_main;

int CALLBACK NewWinMain(
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

auto MAIN_CALL = "53 51 6a 00 68 ?? ?? ?? ?? e8"_pattern;
size_t MAIN_CALL_OFFSET = MAIN_CALL.size;

}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) return true;

    image_base = reinterpret_cast<Byte*>(GetModuleHandle(nullptr));
    auto call_base = FindImage(MAIN_CALL);
    if (!call_base)
    {
        MessageBoxA(nullptr, "Failed to find entry point", nullptr, MB_OK);
        return false;
    }

    auto call = call_base + MAIN_CALL_OFFSET;
    Unprotect up{call, 4};
    orig_main = reinterpret_cast<WinMainPtr>(call + 4 + As<size_t>(call));
    As<size_t>(call) = reinterpret_cast<Byte*>(NewWinMain) - call - 4;

    return true;
}
