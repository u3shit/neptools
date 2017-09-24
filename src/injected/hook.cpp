#include "hook.hpp"
#include <new>
#include <stdexcept>
#include <cstdlib>

#define LIBSHIT_LOG_NAME "hook"
#include <libshit/logger_helper.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Neptools
{

  Byte* image_base;
  static HANDLE heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);

  static constexpr size_t JMP_SIZE = 5;

  size_t GetImageSize() noexcept
  {
    auto dos_hdr = reinterpret_cast<IMAGE_DOS_HEADER*>(image_base);
    auto hdr = reinterpret_cast<IMAGE_NT_HEADERS32*>(image_base + dos_hdr->e_lfanew);
    DBG(1) << "Image size: " << hdr->OptionalHeader.SizeOfImage << std::endl;
    return hdr->OptionalHeader.SizeOfImage;
  }

  Byte* GetEntryPoint() noexcept
  {
    auto dos_hdr = reinterpret_cast<IMAGE_DOS_HEADER*>(image_base);
    auto hdr = reinterpret_cast<IMAGE_NT_HEADERS32*>(image_base + dos_hdr->e_lfanew);
    auto ret = image_base + hdr->OptionalHeader.AddressOfEntryPoint;
    DBG(1) << "OEP: " << ret << std::endl;
    return ret;
  }

  void* Hook(void* fun, void* dst, size_t copy)
  {
    DBG(2) << "Hooking " << fun << " to " << dst << " (" << copy << " bytes)"
           << std::endl;

    char* addr = reinterpret_cast<char*>(fun);

    char* ret = nullptr;
    if (copy)
    {
      ret = static_cast<char*>(HeapAlloc(heap, 0, copy + JMP_SIZE));
      if (!ret) LIBSHIT_THROW(std::bad_alloc, std::make_tuple());
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

    DBG(4) << "done" << std::endl;
    return ret;
  }

  Unprotect::Unprotect(void* ptr, size_t len) : ptr{ptr}, len{len}
  {
    if (!VirtualProtect(ptr, len, PAGE_EXECUTE_READWRITE, &orig_prot))
      LIBSHIT_THROW(std::runtime_error, "Unprotect: VirtualProtect");
  }

  Unprotect::~Unprotect()
  {
    if (!VirtualProtect(ptr, len, orig_prot, &orig_prot))
      MessageBoxA(nullptr, "Failed to unprotect memory", "Neptools",
                  MB_OK | MB_ICONERROR);
  }

}
