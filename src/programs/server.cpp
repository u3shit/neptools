#include "../injected/cpk.hpp"
#include "../injected/hook.hpp"

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

static size_t MAIN_CALL_OFFS = 0x335a0f;
BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH) return true;

    image_base = reinterpret_cast<char*>(GetModuleHandle(nullptr));
    try
    {
        auto call = image_base + MAIN_CALL_OFFS;
        //Unprotect up{call, 4};
        orig_main = reinterpret_cast<WinMainPtr>(call + 4 + As<size_t>(call));
        As<size_t>(call) = reinterpret_cast<char*>(NewWinMain) - call - 4;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "DllMain", MB_OK);
        return false;
    }

    return true;
}
