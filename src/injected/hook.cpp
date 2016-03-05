#include "hook.hpp"
#include <new>
#include <stdexcept>
#include <cstdlib>
#include <boost/algorithm/searching/boyer_moore.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

Byte* image_base;
static HANDLE heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);

static constexpr size_t JMP_SIZE = 5;

static bool CheckPattern(
    const Byte* ptr, const Byte* pattern, const Byte* mask, size_t len)
{
    while (len--)
        if ((*ptr++ & *mask++) != *pattern++)
            return false;
    return true;
}

Byte* Find(const Byte* pattern, const Byte* mask, size_t len) noexcept
{
    size_t max_len = 0, max_i;
    size_t start_i = 0;
    for (size_t i = 0; i < len; ++i)
        if (mask[i] == 0xff)
        {
            if (i - start_i + 1 > max_len)
            {
                max_len = i - start_i + 1;
                max_i = start_i;
            }
        }
        else
            start_i = i+1;

    auto dos_hdr = reinterpret_cast<IMAGE_DOS_HEADER*>(image_base);
    auto hdr = reinterpret_cast<IMAGE_NT_HEADERS32*>(image_base + dos_hdr->e_lfanew);
    auto size = hdr->OptionalHeader.SizeOfImage;

    boost::algorithm::boyer_moore<const Byte*> bm{
        pattern + max_i, pattern + max_i + max_len};

    auto ptr = image_base + max_i;
    auto ptr_end = image_base + size - (len - max_len);
    void* res = nullptr;

    while (true)
    {
        auto match = bm(ptr, ptr_end);
        if (match == ptr_end) break;

        if (CheckPattern(match - max_i, pattern, mask, len))
            if (res)
            {
                std::cerr << "Multiple matches for pattern " << res << " and "
                          << (match - max_i) << std::endl;
                return nullptr;
            }
            else
                res = match - max_i;
        ptr = match + 1;
    }
    return static_cast<Byte*>(res);
}

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
