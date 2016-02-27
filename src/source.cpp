#define _FILE_OFFSET_BITS 64
#include "source.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>

constexpr size_t READ_CHUNK = 8*1024; // 8KiB
constexpr size_t MMAP_CHUNK = 128*1024; // 128KiB
constexpr size_t MMAP_LIMIT = 1*1024*1024; // 1MiB

Source Source::FromFile(fs::path fname)
{
    int fd = open(fname.c_str(), O_RDONLY);
    if (fd == -1)
        throw std::system_error{std::error_code{errno, std::system_category()}};

    struct stat buf;
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }

    std::shared_ptr<SourceProvider> p;
    try { p = std::make_shared<MmapProvider>(fd, fname, buf.st_size); }
    catch (const std::system_error& e)
    {
        std::cerr << "Mmap failed: " << e.what() << std::endl;
        try
        {
            p = std::make_shared<UnixProvider>(
                fd, std::move(fname), buf.st_size);
        }
        catch (...) { close(fd); throw; }
    }
    catch (...) { close(fd); throw; }
    return {std::move(p), static_cast<uint64_t>(buf.st_size)};
}

template <typename T>
Source::UnixLike<T>::~UnixLike()
{
    if (fd != -1) close(fd);
    for (size_t i = 0; i < lru.size(); ++i)
        if (lru[i].size)
            static_cast<T*>(this)->DeleteChunk(i);
}

template <typename T>
void Source::UnixLike<T>::Pread(uint64_t offs, Byte* buf, size_t len)
{
    BOOST_ASSERT(fd != -1);
    if (len > static_cast<T*>(this)->CHUNK_SIZE)
    {
        if (size_t(pread(fd, buf, len, offs)) != len)
            throw std::system_error{std::error_code{errno, std::system_category()}};
        return;
    }

    if (len == 0) EnsureChunk(offs); // TODO: GetTemporaryEntry hack
    while (len)
    {
        EnsureChunk(offs);
        size_t buf_offs = offs - lru[0].offset;
        size_t to_cpy = std::min(len, lru[0].size - buf_offs);
        memcpy(buf, lru[0].ptr + buf_offs, to_cpy);
        buf += to_cpy;
        offs += to_cpy;
        len -= to_cpy;
    }
}

template <typename T>
void Source::UnixLike<T>::EnsureChunk(uint64_t offs)
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

size_t Source::MmapProvider::CHUNK_SIZE = MMAP_CHUNK;
Source::MmapProvider::MmapProvider(int fd, fs::path file_name, uint64_t size)
    : UnixLike{fd, std::move(file_name), size}
{
    size_t to_map = size < MMAP_LIMIT ? size : MMAP_CHUNK;

    auto ptr = mmap(nullptr, to_map, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
        throw std::system_error{std::error_code{errno, std::system_category()}};
    if (to_map == size)
    {
        close(fd);
        this->fd = -1;
    }

    lru[0].ptr = static_cast<Byte*>(ptr);
    lru[0].offset = 0;
    lru[0].size = to_map;
}

void* Source::MmapProvider::ReadChunk(uint64_t offs, size_t size)
{
    auto x = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, offs);
    if (x == MAP_FAILED)
        throw std::system_error{std::error_code{errno, std::system_category()}};
    return x;
}

void Source::MmapProvider::DeleteChunk(size_t i)
{
    munmap(lru[i].ptr, lru[i].size);
}

size_t Source::UnixProvider::CHUNK_SIZE = READ_CHUNK;

void* Source::UnixProvider::ReadChunk(uint64_t offs, size_t size)
{
    auto x = new Byte[size];
    if (size_t(pread(fd, x, size, offs)) != size)
    {
        delete[] x;
        throw std::system_error{std::error_code{errno, std::system_category()}};
    }
    return x;
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
    uint64_t offset = GetOffset();
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
