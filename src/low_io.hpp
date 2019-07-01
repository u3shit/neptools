#ifndef UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#define UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#pragma once

#include "utils.hpp"

#include <libshit/platform.hpp>

namespace Neptools
{

  struct LowIo final
  {
    using FileName =
      const std::conditional_t<LIBSHIT_OS_IS_WINDOWS, wchar_t, char>*;
    using FdType = std::conditional_t<LIBSHIT_OS_IS_WINDOWS, void*, int>;
    static inline const FdType INVALID_FD =
      reinterpret_cast<::Neptools::LowIo::FdType>(-1);

    // used by Source/Sink
    static constexpr const size_t MEM_CHUNK  = 8*1024; // 8KiB
    static constexpr const size_t MMAP_CHUNK = 128*1024; // 128KiB
    static constexpr const size_t MMAP_LIMIT = 1*1024*1024; // 1MiB


    LowIo() noexcept = default;
    explicit LowIo(FdType fd) noexcept : fd{fd} {}
    LowIo(FdType fd, bool owning) noexcept : fd{fd}, owning{owning} {}

    LowIo(FileName fname, bool write);
    ~LowIo() noexcept;

    static LowIo OpenStdOut();

    LowIo(LowIo&& o) noexcept
      : fd{o.fd},
#if LIBSHIT_OS_IS_WINDOWS
        mmap_fd{o.mmap_fd},
#endif
        owning{o.owning}
    {
      o.fd = INVALID_FD;
#if LIBSHIT_OS_IS_WINDOWS
      o.mmap_fd = INVALID_FD;
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

    FdType fd = INVALID_FD;
#if LIBSHIT_OS_IS_WINDOWS
    FdType mmap_fd = INVALID_FD;
#endif
    bool owning = true;
  };

}
#endif
