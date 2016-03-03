#include "source.hpp"
#include "except.hpp"
#include <boost/exception/errinfo_api_function.hpp>
#include <boost/exception/errinfo_file_name.hpp>

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

#include <iostream>
#include <iomanip>

#ifdef WINDOWS
#define SYSERROR(x) \
    THROW(boost::enable_error_info(std::system_error{std::error_code{   \
                    int(GetLastError()), std::system_category()}}) <<   \
        boost::errinfo_api_function{x})
static inline HANDLE Open(const wchar_t* fname)
{
    auto h = CreateFileW(
        fname, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) SYSERROR("CreateFile");
    return h;
}
static inline FilePosition GetSize(HANDLE h)
{
    auto ret = GetFileSize(h, nullptr);
    if (ret == INVALID_FILE_SIZE) SYSERROR("GetFileSize");
    return ret;
}
#define Close CloseHandle

static inline HANDLE PrepareMmap(HANDLE file)
{
    auto map = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, nullptr);
    if (map == nullptr) SYSERROR("CreateFileMapping");
    return map;
}
static inline void* Mmap(HANDLE h, FilePosition offs, FilePosition size)
{
    auto ret = MapViewOfFile(h, FILE_MAP_READ, offs >> 16 >> 16, offs, size);
    if (ret == nullptr) SYSERROR("MapViewOfFile");
    return ret;
}
#define Munmap(ptr,len) UnmapViewOfFile(ptr)
static inline void Pread(HANDLE h, void* buf, FileMemSize len, FilePosition offs)
{
    DWORD size;
    OVERLAPPED o;
    memset(&o, 0, sizeof(OVERLAPPED));
    o.Offset = offs;
    o.OffsetHigh = offs >> 16 >> 16;

    if (!ReadFile(h, buf, len, &size, &o) || size != len) SYSERROR("ReadFile");
}

#else
#define SYSERROR(x)                                                       \
    THROW(boost::enable_error_info(std::system_error{std::error_code{     \
                    errno, std::system_category()}}) <<                   \
        boost::errinfo_api_function{x})
static inline int Open(const char* fname)
{
    int fd = open(fname, O_RDONLY);
    if (fd == -1) SYSERROR("open");
    return fd;
}
static inline FilePosition GetSize(int fd)
{
    struct stat buf;
    if (fstat(fd, &buf) < 0) SYSERROR("fstat");
    return buf.st_size;
}
#define Close close

#define PrepareMmap(fd) fd
static inline void* Mmap(int fd, FilePosition offs, FilePosition size)
{
    auto ptr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, offs);
    if (ptr == MAP_FAILED) SYSERROR("mmap");
    return ptr;
}
#define Munmap munmap

static inline void Pread(int fd, void* buf, FileMemSize len, FilePosition offs)
{
    if (pread(fd, buf, len, offs) != len) SYSERROR("pread");
}
#endif

constexpr size_t READ_CHUNK = 8*1024; // 8KiB
constexpr size_t MMAP_CHUNK = 128*1024; // 128KiB
constexpr size_t MMAP_LIMIT = 1*1024*1024; // 1MiB

Source Source::FromFile(fs::path fname)
{
    return AddInfo(
        &FromFile_,
        [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; },
        fname);
}

Source Source::FromFile_(fs::path fname)
{
    auto fd = Open(fname.c_str());

    FilePosition size;
    try { size = ::GetSize(fd); }
    catch (...) { Close(fd); throw; }

    std::shared_ptr<SourceProvider> p;
    try { p = std::make_shared<MmapProvider>(fd, fname.string(), size); }
    catch (const std::system_error& e)
    {
        std::cerr << "Mmap failed: ";
        PrintException(std::cerr);
        try
        {
            p = std::make_shared<UnixProvider>(
                fd, fname.string(), size);
        }
        catch (...) { Close(fd); throw; }
    }
    catch (...) { Close(fd); throw; }
    return {std::move(p), size};
}

void Source::Pread(FilePosition offs, Byte* buf, FileMemSize len) const
{
    AddInfo([&]
    {
        BOOST_ASSERT(offs <= size && offs+len <= size);
        offs += offset;
        while (len)
        {
            if (GetEntry(offs))
            {
                auto& x = p->lru[0];
                auto buf_offs = offs - x.offset;
                auto to_cpy = std::min(len, x.size - buf_offs);
                memcpy(buf, x.ptr + buf_offs, to_cpy);
                offs += to_cpy;
                buf += to_cpy;
                len -= to_cpy;
            }
            else
                return p->Pread(offs, buf, len);
        }
    },
    [=] (auto& e)
    {
        e << UsedSource{*this} << ReadOffset{offs} << ReadSize{len};
    });
}

template <typename T>
Source::UnixLike<T>::~UnixLike()
{
    if (fd != reinterpret_cast<FdType>(-1)) Close(fd);
    for (size_t i = 0; i < lru.size(); ++i)
        if (lru[i].size)
            static_cast<T*>(this)->DeleteChunk(i);
}

template <typename T>
void Source::UnixLike<T>::Pread(FilePosition offs, Byte* buf, FileMemSize len)
{
    BOOST_ASSERT(fd != reinterpret_cast<FdType>(-1));
    if (len > static_cast<T*>(this)->CHUNK_SIZE)
        return ::Pread(fd, buf, len, offs);

    if (len == 0) EnsureChunk(offs); // TODO: GetTemporaryEntry hack
    while (len)
    {
        EnsureChunk(offs);
        auto buf_offs = offs - lru[0].offset;
        auto to_cpy = std::min(len, lru[0].size - buf_offs);
        memcpy(buf, lru[0].ptr + buf_offs, to_cpy);
        buf += to_cpy;
        offs += to_cpy;
        len -= to_cpy;
    }
}

template <typename T>
void Source::UnixLike<T>::EnsureChunk(FilePosition offs)
{
    auto const CHUNK_SIZE = static_cast<T*>(this)->CHUNK_SIZE;
    auto ch_offs = offs/CHUNK_SIZE*CHUNK_SIZE;
    for (size_t i = 0; i < lru.size(); ++i)
        if (lru[i].offset == ch_offs)
        {
            auto x = lru[i];
            memmove(&lru[1], &lru[0], sizeof(BufEntry)*i);
            lru[0] = x;
            return;
        }

    auto size = std::min(CHUNK_SIZE, this->size-ch_offs);
    auto x = static_cast<T*>(this)->ReadChunk(ch_offs, size);
    static_cast<T*>(this)->DeleteChunk(lru.size()-1);
    memmove(&lru[1], &lru[0], sizeof(BufEntry)*(lru.size()-1));

    lru[0].ptr = static_cast<Byte*>(x);
    lru[0].offset = ch_offs;
    lru[0].size = size;
}

FileMemSize Source::MmapProvider::CHUNK_SIZE = MMAP_CHUNK;
Source::MmapProvider::MmapProvider(
    FdType in_fd, fs::path file_name, FilePosition size)
    : UnixLike{PrepareMmap(in_fd), std::move(file_name), size}
{
    size_t to_map = size < MMAP_LIMIT ? size : MMAP_CHUNK;

#ifdef WINDOWS
    real_fd = in_fd;
    void* ptr;
    try { ptr = Mmap(fd, 0, to_map); }
    catch(...) { Close(fd); throw; }
#else
    auto ptr = Mmap(fd, 0, to_map);
    if (to_map == size)
    {
        close(fd);
        this->fd = -1;
    }
#endif

    lru[0].ptr = static_cast<Byte*>(ptr);
    lru[0].offset = 0;
    lru[0].size = to_map;
}

Source::MmapProvider::~MmapProvider()
{
#ifdef WINDOWS
    Close(real_fd);
#endif
}

void* Source::MmapProvider::ReadChunk(FilePosition offs, FileMemSize size)
{
    return Mmap(fd, offs, size);
}

void Source::MmapProvider::DeleteChunk(size_t i)
{
    if (lru[i].ptr)
        Munmap(lru[i].ptr, lru[i].size);
}

FilePosition Source::UnixProvider::CHUNK_SIZE = READ_CHUNK;

void* Source::UnixProvider::ReadChunk(FilePosition offs, FileMemSize size)
{
    std::unique_ptr<Byte[]> x{new Byte[size]};
    ::Pread(fd, x.get(), size, offs);
    return x.release();
}

void Source::UnixProvider::DeleteChunk(size_t i)
{
    delete[] lru[i].ptr;
}

void DumpableSource::Inspect_(std::ostream& os) const
{
    auto flags = os.flags();
    os << std::hex << "bin(";
    DumpBytes(os, GetFileName().generic_string());
    os << ", 0x" << GetOffset() << ", 0x" << GetSize() << ")";
    os.flags(flags);
}

void DumpableSource::Dump_(std::ostream& os) const
{
    auto offset = GetOffset();
    auto rem_size = GetSize();
    while (rem_size)
    {
        auto x = GetTemporaryEntry(offset);
        BOOST_ASSERT(x.offset <= offset);
        auto ptroff = offset - x.offset;
        auto size = std::min(rem_size, x.size - ptroff);
        os.write(reinterpret_cast<char*>(x.ptr + ptroff), size);
        offset += size;
        rem_size -= size;
    }
}

std::string to_string(const UsedSource& src)
{
    std::stringstream ss;
    ss << "[Source] = ";
    DumpableSource{src.value()}.Inspect(ss);
    ss << ", pos: " << src.value().Tell() << '\n';
    return ss.str();
}
