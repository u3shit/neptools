#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define MSVC_URL "https://www.microsoft.com/en-us/download/details.aspx?id=49984"

// set shit that's possible here
#pragma comment(linker, "/merge:.text=.data")
#pragma comment(linker, "/merge:.rdata=.data")

static void strwcpy(wchar_t* dst, const wchar_t* src)
{
    while (*dst++ = *src++);
}

void* memset(void* dst, int c, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        ((char *) dst)[i] = c;
    return dst;
}

static size_t strwlen(const wchar_t* ptr)
{
    const wchar_t* p = ptr;
    while (*p++);
    return p - ptr;
}

static int inject_dll(HANDLE proc, wchar_t* fname)
{
    size_t bytes = sizeof(wchar_t) * (strwlen(fname)+1);
    void* ptr = VirtualAllocEx(proc, NULL, bytes, MEM_COMMIT, PAGE_READWRITE);
    if (!ptr) return 0;

    if (!WriteProcessMemory(proc, ptr, fname, bytes, NULL)) return 0;

    HANDLE th = CreateRemoteThread(
        proc, NULL, 0, (LPTHREAD_START_ROUTINE) &LoadLibraryW, ptr, 0, NULL);
    if (!th) return 0;

    WaitForSingleObject(th, INFINITE);
    DWORD ret;
    if (!GetExitCodeThread(th, &ret) || ret == 0) return 0;
    CloseHandle(th);

    if (!VirtualFreeEx(proc, ptr, 0, MEM_RELEASE)) return 0;

    return 1;
}

static int mymain(void)
{
    HANDLE h = LoadLibraryExA("msvcp140.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (h == NULL)
    {
        int ret = MessageBoxA(
            NULL, "Please install MSVC 2015 runtime\n\nOpen download page?",
            NULL, MB_YESNO | MB_ICONERROR);
        if (ret == IDYES)
            ShellExecuteA(NULL, NULL, MSVC_URL, NULL, NULL, SW_SHOWNORMAL);
        return 1;
    }
    CloseHandle(h);

#define BUF_SIZE 4096
    wchar_t buf[BUF_SIZE];
    GetModuleFileNameW(NULL, buf, BUF_SIZE);
    buf[BUF_SIZE-1] = 0; // maybe xp

    wchar_t* last_slash = buf;
    for (wchar_t* ptr = buf; *ptr; ++ptr)
        if (*ptr == L'\\')
            last_slash = ptr;

    if (*last_slash != L'\\' || last_slash - buf > BUF_SIZE - 22)
    {
        MessageBoxA(NULL, "Wrong module path", NULL, MB_OK | MB_ICONERROR);
        return 2;
    }
    strwcpy(last_slash + 1, L"NeptuniaReBirth3.exe");

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(STARTUPINFO);
    if (CreateProcessW(buf, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL,
                       NULL, &si, &pi) == 0)
    {
        MessageBoxA(NULL, "Failed to start game", NULL, MB_OK | MB_ICONERROR);
        return 2;
    }

    strwcpy(last_slash + 1, L"server.dll");
    if (!inject_dll(pi.hProcess, buf))
    {
        TerminateProcess(pi.hProcess, 0);
        MessageBoxA(NULL, "DLL injection failed", NULL, MB_OK | MB_ICONERROR);
        return 3;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

void __cdecl start()
{
    ExitProcess(mymain());
}
