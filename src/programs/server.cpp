#include "../injected/cpk.hpp"
#include "../injected/hook.hpp"
#include "../injected/pattern_parse.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using WinMainPtr = int (CALLBACK*)(HINSTANCE, HINSTANCE, wchar_t*, int);
static WinMainPtr orig_main;

#include <iostream>
static int CALLBACK NewWinMain(
    HINSTANCE inst, HINSTANCE prev, wchar_t* cmdline, int show_cmd)
{
    try
    {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        std::cerr << "Console init" << std::endl;
        CpkHandler::Hook();
        std::cerr << "Hook done" << std::endl;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "WinMain", MB_OK);
        return -1;
    }


    std::cerr << "Starting main" << std::endl;
    return orig_main(inst, prev, cmdline, show_cmd);
}

auto MAIN_CALL = "53 51 6a 00 68 ?? ?? ?? ?? e8"_pattern;
static size_t MAIN_CALL_OFFSET = MAIN_CALL.size;

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) return true;

    image_base = reinterpret_cast<Byte*>(GetModuleHandle(nullptr));
    auto call_base = Find(MAIN_CALL);
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
