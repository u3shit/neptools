#include "source.hpp"
#include "except.hpp"
#include <boost/exception/errinfo_file_name.hpp>
#include <iostream>

namespace
{

template <typename T>
struct UnixLike : public Source::Provider
{
    UnixLike(LowIo io, fs::path file_name, FilePosition size)
        : Source::Provider{std::move(file_name), size}, io{std::move(io)} {}
    ~UnixLike();

    void Pread(FilePosition offs, Byte* buf, FileMemSize len) override;
    void EnsureChunk(FilePosition i);

    LowIo io;
};

struct MmapProvider final : public UnixLike<MmapProvider>
{
    MmapProvider(LowIo&& fd, fs::path file_name, FilePosition size);

    static FileMemSize CHUNK_SIZE;
    void* ReadChunk(FilePosition offs, FileMemSize size);
    void DeleteChunk(size_t i);
};

struct UnixProvider final : public UnixLike<UnixProvider>
{
    using UnixLike::UnixLike;

    static FileMemSize CHUNK_SIZE;
    void* ReadChunk(FilePosition offs, FileMemSize size);
    void DeleteChunk(size_t i);
};

}


Source Source::FromFile(fs::path fname)
{
    return AddInfo(
        &FromFile_,
        [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; },
        fname);
}

Source Source::FromFile_(fs::path fname)
{
    LowIo io{fname.c_str(), false};

    FilePosition size = io.GetSize();

    std::shared_ptr<Provider> p;
    try { p = std::make_shared<MmapProvider>(std::move(io), fname.string(), size); }
    catch (const std::system_error& e)
    {
        std::cerr << "Mmap failed: ";
        PrintException(std::cerr);
        p = std::make_shared<UnixProvider>(std::move(io), fname.string(), size);
    }
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
UnixLike<T>::~UnixLike()
{
    for (size_t i = 0; i < lru.size(); ++i)
        if (lru[i].size)
            static_cast<T*>(this)->DeleteChunk(i);
}

template <typename T>
void UnixLike<T>::Pread(FilePosition offs, Byte* buf, FileMemSize len)
{
    BOOST_ASSERT(io.fd != reinterpret_cast<FdType>(-1));
    if (len > static_cast<T*>(this)->CHUNK_SIZE)
        return io.Pread(buf, len, offs);

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
void UnixLike<T>::EnsureChunk(FilePosition offs)
{
    auto const CHUNK_SIZE = static_cast<T*>(this)->CHUNK_SIZE;
    auto ch_offs = offs/CHUNK_SIZE*CHUNK_SIZE;
    for (size_t i = 0; i < lru.size(); ++i)
        if (lru[i].offset == ch_offs)
        {
            auto x = lru[i];
            memmove(&lru[1], &lru[0], sizeof(Source::BufEntry)*i);
            lru[0] = x;
            return;
        }

    auto size = std::min(CHUNK_SIZE, this->size-ch_offs);
    auto x = static_cast<T*>(this)->ReadChunk(ch_offs, size);
    static_cast<T*>(this)->DeleteChunk(lru.size()-1);
    memmove(&lru[1], &lru[0], sizeof(Source::BufEntry)*(lru.size()-1));

    lru[0].ptr = static_cast<Byte*>(x);
    lru[0].offset = ch_offs;
    lru[0].size = size;
}

FileMemSize MmapProvider::CHUNK_SIZE = MMAP_CHUNK;
MmapProvider::MmapProvider(
    LowIo&& io, fs::path file_name, FilePosition size)
    : UnixLike{{}, std::move(file_name), size}
{
    size_t to_map = size < MMAP_LIMIT ? size : MMAP_CHUNK;

    io.PrepareMmap(false);
    void* ptr = io.Mmap(0, to_map, false);
#ifndef WINDOWS
    if (to_map == size)
    {
        close(io.fd);
        io.fd = -1;
    }
#endif
    this->io = std::move(io);

    lru[0].ptr = static_cast<Byte*>(ptr);
    lru[0].offset = 0;
    lru[0].size = to_map;
}

void* MmapProvider::ReadChunk(FilePosition offs, FileMemSize size)
{
    return io.Mmap(offs, size, false);
}

void MmapProvider::DeleteChunk(size_t i)
{
    if (lru[i].ptr)
        io.Munmap(lru[i].ptr, lru[i].size);
}

FilePosition UnixProvider::CHUNK_SIZE = MEM_CHUNK;

void* UnixProvider::ReadChunk(FilePosition offs, FileMemSize size)
{
    std::unique_ptr<Byte[]> x{new Byte[size]};
    io.Pread(x.get(), size, offs);
    return x.release();
}

void UnixProvider::DeleteChunk(size_t i)
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

void DumpableSource::Dump_(Sink& sink) const
{
    auto offset = GetOffset();
    auto rem_size = GetSize();
    while (rem_size)
    {
        auto x = GetTemporaryEntry(offset);
        BOOST_ASSERT(x.offset <= offset);
        auto ptroff = offset - x.offset;
        auto size = std::min(rem_size, x.size - ptroff);
        sink.Write(x.ptr + ptroff, size);
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
