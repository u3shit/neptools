#ifndef UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#define UUID_0B3D6321_5A5C_4A11_AA24_B815F3DE798C
#pragma once

#include "utils.hpp"

#ifdef WINDOWS
using FileName = const wchar_t*;
using FdType = void*;
static const FdType INVALID_FD = reinterpret_cast<FdType>(-1);
#else
using FileName = const char*;
using FdType = int;
static const FdType INVALID_FD = -1;
#endif

// used by Source/Sink
constexpr const size_t MEM_CHUNK  = 8*1024; // 8KiB
constexpr const size_t MMAP_CHUNK = 128*1024; // 128KiB
constexpr const size_t MMAP_LIMIT = 1*1024*1024; // 1MiB

struct LowIo final
{
    LowIo() : fd{INVALID_FD} {}
    explicit LowIo(FdType fd) : fd{fd} {}
    LowIo(FileName fname, bool write);
    ~LowIo();

    static LowIo OpenStdOut();

    LowIo(LowIo&& o)
        : fd{o.fd}
#ifdef WINDOWS
        , mmap_fd{o.mmap_fd}
#endif
    {
        o.fd = INVALID_FD;
#ifdef WINDOWS
        o.mmap_fd = INVALID_FD;
#endif
    }
    LowIo& operator=(LowIo&& o)
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
    FdType mmap_fd = INVALID_FD;
#endif
};

using MmapOffset = boost::error_info<struct MmapOffsetTag, FilePosition>;
using MmapSize = boost::error_info<struct MmapSizeTag, FilePosition>;

#endif
