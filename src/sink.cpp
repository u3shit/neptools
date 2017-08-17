#include "sink.hpp"
#include "low_io.hpp"

#include <libshit/except.hpp>
#include <libshit/lua/boost_endian_traits.hpp>

#include <iostream>
#include <boost/exception/errinfo_file_name.hpp>

#define LIBSHIT_LOG_NAME "sink"
#include <libshit/logger_helper.hpp>

namespace Neptools
{
namespace
{

struct LIBSHIT_NOLUA MmapSink final : public Sink
{
    MmapSink(LowIo&& io, FilePosition size);
    ~MmapSink();
    void Write_(Libshit::StringView data) override;
    void Pad_(FileMemSize len) override;

    void MapNext(FileMemSize len);

    LowIo io;
};

struct LIBSHIT_NOLUA SimpleSink final : public Sink
{
    SimpleSink(LowIo io, FilePosition size) : Sink{size}, io{std::move(io)}
    {
        Sink::buf = buf;
        buf_size = LowIo::MEM_CHUNK;
    }
    ~SimpleSink();

    void Write_(Libshit::StringView data) override;
    void Pad_(FileMemSize len) override;
    void Flush() override;

    LowIo io;
    Byte buf[LowIo::MEM_CHUNK];
};

}

MmapSink::MmapSink(LowIo&& io, FilePosition size) : Sink{size}
{
    size_t to_map = size < LowIo::MMAP_LIMIT ? size : LowIo::MMAP_CHUNK;

    io.Truncate(size);
    io.PrepareMmap(true);
    buf_size = to_map;
    buf = static_cast<Byte*>(io.Mmap(0, to_map, true));

    this->io = std::move(io);
}

MmapSink::~MmapSink()
{
    if (buf)
        LowIo::Munmap(buf, buf_size);
}

void MmapSink::Write_(Libshit::StringView data)
{
    LIBSHIT_ASSERT(buf_put == buf_size && offset < size &&
                    buf_size == LowIo::MMAP_CHUNK);

    offset += buf_put;
    if (data.length() / LowIo::MMAP_CHUNK)
    {
        auto to_write = data.length() / LowIo::MMAP_CHUNK * LowIo::MMAP_CHUNK;
        io.Pwrite(data.udata(), to_write, offset);
        data.remove_prefix(to_write);
        offset += to_write;
        buf_put = 0;
    }

    MapNext(data.length());
    if (buf) // https://stackoverflow.com/a/5243068; C99 7.21.1/2
        memcpy(buf, data.data(), data.length());
    else
        LIBSHIT_ASSERT(data.length() == 0);
}

void MmapSink::Pad_(FileMemSize len)
{
    LIBSHIT_ASSERT(buf_put == buf_size && offset < size &&
                    buf_size == LowIo::MMAP_CHUNK);

    offset += buf_put + len / LowIo::MMAP_CHUNK * LowIo::MMAP_CHUNK;
    LIBSHIT_ASSERT_MSG(offset <= size, "sink overflow");
    MapNext(len % LowIo::MMAP_CHUNK);
}

void MmapSink::MapNext(FileMemSize len)
{
    // wine fails on 0 size
    // windows fails if offset+size > file_length...
    // (linux doesn't care...)
    if (offset < size)
    {
        auto nbuf_size = std::min<FileMemSize>(LowIo::MMAP_CHUNK, size-offset);
        LIBSHIT_ASSERT(nbuf_size >= len);
        void* nbuf = io.Mmap(offset, nbuf_size, true);
        io.Munmap(buf, buf_size);
        buf = static_cast<Byte*>(nbuf);
        buf_put = len;
        buf_size = nbuf_size;
    }
    else
    {
        io.Munmap(buf, buf_size);
        buf = nullptr;
    }
}

SimpleSink::~SimpleSink()
{
    try { Flush(); }
    catch (std::exception& e)
    {
        ERR << "~SimpleSink " << Libshit::ExceptionToString() << std::endl;
    }
}

void SimpleSink::Flush()
{
    if (buf_put)
    {
        io.Write(buf, buf_put);
        offset += buf_put;
        buf_put = 0;
    }
}

void SimpleSink::Write_(Libshit::StringView data)
{
    LIBSHIT_ASSERT(buf_size == LowIo::MEM_CHUNK &&
                    buf_put == LowIo::MEM_CHUNK);
    io.Write(buf, LowIo::MEM_CHUNK);
    offset += LowIo::MEM_CHUNK;

    if (data.length() >= LowIo::MEM_CHUNK)
    {
        io.Write(data.data(), data.length());
        offset += data.length();
        buf_put = 0;
    }
    else
    {
        memcpy(buf, data.data(), data.length());
        buf_put = data.length();
    }
}

void SimpleSink::Pad_(FileMemSize len)
{
    LIBSHIT_ASSERT(buf_size == LowIo::MEM_CHUNK &&
                    buf_put == LowIo::MEM_CHUNK);
    io.Write(buf, LowIo::MEM_CHUNK);
    offset += LowIo::MEM_CHUNK;

    // assume we're not seekable (I don't care about not mmap-able but seekable
    // files)
    if (len >= LowIo::MEM_CHUNK)
    {
        memset(buf, 0, LowIo::MEM_CHUNK);
        size_t i;
        for (i = LowIo::MEM_CHUNK; i < len; i += LowIo::MEM_CHUNK)
            io.Write(buf, LowIo::MEM_CHUNK);
        offset += i - LowIo::MEM_CHUNK;
    }
    else
        memset(buf, 0, len);
    buf_put = len % LowIo::MEM_CHUNK;
}

Libshit::NotNull<Libshit::RefCountedPtr<Sink>> Sink::ToFile(
    boost::filesystem::path fname, FilePosition size, bool try_mmap)
{
    return Libshit::AddInfo(
        [&]() -> Libshit::NotNull<Libshit::RefCountedPtr<Sink>>
        {
            LowIo io{fname.c_str(), true};
            if (!try_mmap)
                return Libshit::MakeRefCounted<SimpleSink>(std::move(io), size);

            try { return Libshit::MakeRefCounted<MmapSink>(std::move(io), size); }
            catch (const std::system_error& e)
            {
                WARN << "Mmmap failed, falling back to normal writing: "
                     << Libshit::ExceptionToString() << std::endl;
                return Libshit::MakeRefCounted<SimpleSink>(std::move(io), size);
            }
        },
        [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; });
}

Libshit::NotNull<Libshit::RefCountedPtr<Sink>> Sink::ToStdOut()
{
    return Libshit::MakeRefCounted<SimpleSink>(LowIo::OpenStdOut(), -1);
}

void MemorySink::Write_(Libshit::StringView)
{ LIBSHIT_UNREACHABLE("MemorySink::Write_ called"); }
void MemorySink::Pad_(FileMemSize)
{ LIBSHIT_UNREACHABLE("MemorySink::Pad_ called"); }

#ifndef LIBSHIT_WITHOUT_LUA
LIBSHIT_LUAGEN(name="new", class="Neptools::MemorySink")
static Libshit::NotNull<Libshit::SmartPtr<MemorySink>>
MemorySinkFromLua(Libshit::StringView view)
{
    std::unique_ptr<Byte[]> buf{new Byte[view.length()]};
    memcpy(buf.get(), view.data(), view.length());
    return Libshit::MakeSmart<MemorySink>(std::move(buf), view.length());
}
#endif

}

#include "sink.binding.hpp"
