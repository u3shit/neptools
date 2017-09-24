#ifndef UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#define UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#pragma once

#include "utils.hpp"

namespace Neptools
{

  struct LowIo final
  {
#ifdef WINDOWS
    using FileName = const wchar_t*;
    using FdType = void*;
#define NEPTOOLS_INVALID_FD reinterpret_cast<::Neptools::LowIo::FdType>(-1)
#else
    using FileName = const char*;
    using FdType = int;
#define NEPTOOLS_INVALID_FD (-1)
#endif

    // used by Source/Sink
    static constexpr const size_t MEM_CHUNK  = 8*1024; // 8KiB
    static constexpr const size_t MMAP_CHUNK = 128*1024; // 128KiB
    static constexpr const size_t MMAP_LIMIT = 1*1024*1024; // 1MiB


    LowIo() : fd{NEPTOOLS_INVALID_FD} {}
    explicit LowIo(FdType fd) : fd{fd} {}
    LowIo(FileName fname, bool write);
    ~LowIo() noexcept;

    static LowIo OpenStdOut();

    LowIo(LowIo&& o) noexcept
        : fd{o.fd}
#ifdef WINDOWS
        , mmap_fd{o.mmap_fd}
#endif
    {
      o.fd = NEPTOOLS_INVALID_FD;
#ifdef WINDOWS
      o.mmap_fd = NEPTOOLS_INVALID_FD;
#endif
    }
    LowIo& operator=(LowIo&& o) noexcept
    {
      this->~LowIo();
      new (this) LowIo{std::move(o)};
      return *this;
    }

    FilePosition GetSize() const;
    void Truncate(FilePosition size) const;
    void PrepareMmap(bool write);
    void* Mmap(FilePosition offs, FileMemSize size, bool write) const;
    static void Munmap(void* ptr, FileMemSize len);
    void Pread(void* buf, FileMemSize len, FilePosition offs) const;
    void Pwrite(const void* buf, FileMemSize len, FilePosition offs) const;
    void Write(const void* buf, FileMemSize len) const;

    FdType fd;
#ifdef WINDOWS
    FdType mmap_fd = NEPTOOLS_INVALID_FD;
#endif
  };

}
#endif
