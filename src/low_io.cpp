#include "low_io.hpp"

#include <libshit/except.hpp>

#if LIBSHIT_OS_IS_WINDOWS
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <iostream>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  if !LIBSHIT_OS_IS_VITA
#    include <sys/mman.h>
#  endif
#endif

namespace Neptools
{

// common helpers
#if LIBSHIT_OS_IS_WINDOWS
#  define SYSERROR2(x, ...)                                \
  LIBSHIT_THROW(Libshit::SystemError, std::error_code{     \
      int(GetLastError()), std::system_category()},        \
    "API function", x __VA_ARGS__)
#  define SYSERROR(x) SYSERROR2(x, )

  LowIo::LowIo(const wchar_t* fname, bool write)
    : fd{CreateFileW(
        fname, write ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
        FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr,
        write ? CREATE_ALWAYS : OPEN_EXISTING, 0,
        nullptr)}
  {
    if (fd == INVALID_HANDLE_VALUE) SYSERROR("CreateFile");
  }

  LowIo LowIo::OpenStdOut()
  {
    auto h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == nullptr) SYSERROR("GetStdHandle");
    HANDLE ret;
    if (DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &ret, 0,
                        FALSE, DUPLICATE_SAME_ACCESS) == 0)
      SYSERROR("DuplicateHandle");
    return LowIo{ret};
  }

  LowIo::~LowIo() noexcept
  {
    CloseHandle(mmap_fd);
    if (owning) CloseHandle(fd);
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
      SYSERROR2("MapViewOfFile", ,"Mmap offset", offs, "Mmap size", size);
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

#  define SYSERROR(x)                                           \
  LIBSHIT_THROW(Libshit::SystemError,                           \
                std::error_code{errno, std::system_category()}, \
                "API function", x)

  LowIo::LowIo(const char* fname, bool write)
    : fd{open(fname, write ? O_CREAT | O_TRUNC | O_RDWR : O_RDONLY, 0666)}
  {
    if (fd == -1) SYSERROR("open");
  }

  LowIo LowIo::OpenStdOut()
  {
    int fd = dup(1);
    if (fd == -1) SYSERROR("dup");
    return LowIo{fd};
  }

  LowIo::~LowIo() noexcept
  {
    if (owning && fd != -1 && close(fd) != 0)
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
#if LIBSHIT_OS_IS_VITA
    errno = ENOSYS;
    SYSERROR("mmap");
#else
    auto ptr = mmap(
      nullptr, size, write ? PROT_WRITE : PROT_READ,
      write ? MAP_SHARED : MAP_PRIVATE, fd, offs);
    if (ptr == MAP_FAILED) SYSERROR("mmap");
    return ptr;
#endif
  }

  void LowIo::Munmap(void* ptr, FileMemSize len)
  {
#if !LIBSHIT_OS_IS_VITA
    if (munmap(ptr, len) != 0)
    {
      perror("munmap");
      abort();
    }
#endif
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
}
