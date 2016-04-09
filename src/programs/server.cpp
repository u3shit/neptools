#include "../injected/cpk.hpp"
#include "../injected/hook.hpp"
#include "../pattern_parse.hpp"

#define NEPTOOLS_LOG_NAME "server"
#include "../logger_helper.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

using WinMainPtr = int (CALLBACK*)(HINSTANCE, HINSTANCE, wchar_t*, int);
static WinMainPtr orig_main;

#include <iostream>
static int CALLBACK NewWinMain(
    HINSTANCE inst, HINSTANCE prev, wchar_t* cmdline, int show_cmd)
{
    try
    {
#ifndef NDEBUG
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        INFO << "Console init" << std::endl;
#endif
        CpkHandler::Init();
        DBG(0) << "Hook done" << std::endl;
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
static size_t MAIN_CALL_OFFSET = MAIN_CALL.size;

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
