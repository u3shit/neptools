#include "low_io.hpp"
#include "except.hpp"
#include <boost/exception/errinfo_api_function.hpp>

#ifdef WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

// common helpers
#ifdef WINDOWS
#include <iostream>

#define SYSERROR2(x, rst)                                               \
    THROW(boost::enable_error_info(std::system_error{std::error_code{   \
                    int(GetLastError()), std::system_category()}}) <<   \
        boost::errinfo_api_function{x} rst)
#define SYSERROR(x) SYSERROR2(x, )

LowIo::LowIo(const wchar_t* fname, bool write)
    : fd{CreateFileW(
        fname, write ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr,
        write ? CREATE_ALWAYS : OPEN_EXISTING, 0,
        nullptr)}
{
    if (fd == INVALID_HANDLE_VALUE) SYSERROR("CreateFile");
}

LowIo::~LowIo()
{
    CloseHandle(mmap_fd);
    CloseHandle(fd);
}

FilePosition LowIo::GetSize() const
{
    auto ret = GetFileSize(fd, nullptr);
    if (ret == INVALID_FILE_SIZE) SYSERROR("GetFileSize");
    return ret;
}

void LowIo::Truncate(FilePosition size) const
{
    auto old = SetFilePointer(fd, 0, nullptr, FILE_CURRENT);
    if (old ==  INVALID_SET_FILE_POINTER) SYSERROR("SetFilePointer");
    if (SetFilePointer(fd, size, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        SYSERROR("SetFilePointer");
    if (SetEndOfFile(fd) == 0) SYSERROR("SetEndOfFile");
    SetFilePointer(fd, old, nullptr, FILE_BEGIN);
}

void LowIo::PrepareMmap(bool write)
{
    mmap_fd = CreateFileMapping(
        fd, NULL, write ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
    if (mmap_fd == nullptr) SYSERROR("CreateFileMapping");
}

void* LowIo::Mmap(FilePosition offs, FilePosition size, bool write) const
{
    auto ret = MapViewOfFile(mmap_fd, write ? FILE_MAP_WRITE : FILE_MAP_READ,
                             offs >> 16 >> 16, offs, size);
    if (ret == nullptr)
        SYSERROR2("MapViewOfFile", << MmapOffset(offs) << MmapSize(size));
    return ret;
}

void LowIo::Munmap(void* ptr, FileMemSize)
{
    if (UnmapViewOfFile(ptr) == 0)
        abort();
}
void LowIo::Pread(void* buf, FileMemSize len, FilePosition offs) const
{
    DWORD size;
    OVERLAPPED o;
    memset(&o, 0, sizeof(OVERLAPPED));
    o.Offset = offs;
    o.OffsetHigh = offs >> 16 >> 16;

    if (!ReadFile(fd, buf, len, &size, &o) || size != len)
        SYSERROR("ReadFile");
}

void LowIo::Pwrite(const void* buf, FileMemSize len, FilePosition offs) const
{
    DWORD size;
    OVERLAPPED o;
    memset(&o, 0, sizeof(OVERLAPPED));
    o.Offset = offs;
    o.OffsetHigh = offs >> 16 >> 16;

    if (!WriteFile(fd, buf, len, &size, &o) || size != len)
        SYSERROR("WriteFile");
}

void LowIo::Write(const void* buf, FileMemSize len) const
{
    DWORD size;
    if (!WriteFile(fd, buf, len, &size, nullptr) || size != len)
        SYSERROR("WriteFile");
}

#else // linux/unix

#define SYSERROR(x)                                                       \
    THROW(boost::enable_error_info(std::system_error{std::error_code{     \
                    errno, std::system_category()}}) <<                   \
        boost::errinfo_api_function{x})

LowIo::LowIo(const char* fname, bool write)
    : fd{open(fname, write ? O_CREAT | O_TRUNC | O_RDWR : O_RDONLY, 0666)}
{
    if (fd == -1) SYSERROR("open");
}

LowIo::~LowIo()
{
    if (fd != -1 && close(fd) != 0)
        perror("close");
}

FilePosition LowIo::GetSize() const
{
    struct stat buf;
    if (fstat(fd, &buf) < 0) SYSERROR("fstat");
    return buf.st_size;
}

void LowIo::Truncate(FilePosition size) const
{
    if (ftruncate(fd, size) < 0) SYSERROR("ftruncate");
}

void LowIo::PrepareMmap(bool) {}

void* LowIo::Mmap(FilePosition offs, FilePosition size, bool write) const
{
    auto ptr = mmap(
        nullptr, size, write ? PROT_WRITE : PROT_READ,
        write ? MAP_SHARED : MAP_PRIVATE, fd, offs);
    if (ptr == MAP_FAILED) SYSERROR("mmap");
    return ptr;
}

void LowIo::Munmap(void* ptr, FileMemSize len)
{
    if (munmap(ptr, len) != 0)
    {
        perror("munmap");
        abort();
    }
}

void LowIo::Pread(void* buf, FileMemSize len, FilePosition offs) const
{
    if (pread(fd, buf, len, offs) != len) SYSERROR("pread");
}

void LowIo::Pwrite(const void* buf, FileMemSize len, FilePosition offs) const
{
    if (pwrite(fd, buf, len, offs) != len) SYSERROR("pwrite");
}

void LowIo::Write(const void* buf, FileMemSize len) const
{
    if (write(fd, buf, len) != len) SYSERROR("write");
}
#endif
