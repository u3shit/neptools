#include "hook.hpp"
#include "../except.hpp"
#include <new>
#include <stdexcept>
#include <cstdlib>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

char* image_base;
static HANDLE heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);

static constexpr size_t JMP_SIZE = 5;

void* Hook(void* fun, void* dst, size_t copy)
{
    char* addr = reinterpret_cast<char*>(fun);

    char* ret = nullptr;
    if (copy)
    {
        ret = static_cast<char*>(HeapAlloc(heap, 0, copy + JMP_SIZE));
        if (!ret) THROW(std::bad_alloc{});
        memcpy(ret, addr, copy);
        ret[copy] = 0xe9; // jmp
        auto base = ret + copy + JMP_SIZE;
        auto tgt = addr + copy;
        As<int>(ret+copy+1) = tgt - base;
    }

    try
    {
        Unprotect up{addr, 5};
        addr[0] = 0xe9; // jmp
        As<int>(addr+1) = reinterpret_cast<char*>(dst) - addr - JMP_SIZE;
    }
    catch (...)
    {
        if (ret) HeapFree(heap, 0, ret);
        throw;
    }

    return ret;
}

Unprotect::Unprotect(void* ptr, size_t len) : ptr{ptr}, len{len}
{
    if (!VirtualProtect(ptr, len, PAGE_READWRITE, &orig_prot))
        THROW(std::runtime_error{"Unprotect: VirtualProtect"});
}

Unprotect::~Unprotect()
{
    if (!VirtualProtect(ptr, len, orig_prot, &orig_prot))
        abort();
}
