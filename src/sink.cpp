#include "sink.hpp"
#include "low_io.hpp"
#include "except.hpp"
#include <boost/exception/errinfo_file_name.hpp>
#include <iostream>

namespace
{

struct MmapSink : public Sink
{
    MmapSink(LowIo&& io, FilePosition size);
    ~MmapSink();
    void Write_(const Byte* buf, FileMemSize len) override;
    void Pad_(FileMemSize len) override;

    void MapNext(FileMemSize len);

    LowIo io;
};

struct SimpleSink : public Sink
{
    SimpleSink(LowIo io, FilePosition size) : Sink{size}, io{std::move(io)}
    {
        Sink::buf = buf;
        buf_size = MEM_CHUNK;
    }
    ~SimpleSink();

    void Write_(const Byte* buf, FileMemSize len) override;
    void Pad_(FileMemSize len) override;
    void Flush() override;

    LowIo io;
    Byte buf[MEM_CHUNK];
};

}

MmapSink::MmapSink(LowIo&& io, FilePosition size) : Sink{size}
{
    size_t to_map = size < MMAP_LIMIT ? size : MMAP_CHUNK;

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

void MmapSink::Write_(const Byte* data, FileMemSize len)
{
    BOOST_ASSERT(buf_put == buf_size && offset < size && buf_size == MMAP_CHUNK);

    offset += buf_put;
    if (len / MMAP_CHUNK)
    {
        auto to_write = len / MMAP_CHUNK * MMAP_CHUNK;
        io.Pwrite(data, to_write, offset);
        data += to_write;
        offset += to_write;
        len -= to_write;
        buf_put = 0;
    }

    MapNext(len);
    memcpy(buf, data, len);
    buf_put = len;
}

void MmapSink::Pad_(FileMemSize len)
{
    BOOST_ASSERT(buf_put == buf_size && offset < size && buf_size == MMAP_CHUNK);

    offset += buf_put + len / MMAP_CHUNK * MMAP_CHUNK;
    MapNext(len % MMAP_CHUNK);
}

void MmapSink::MapNext(FileMemSize len)
{
    // wine fails on 0 size
    // windows fails if offset+size > file_length...
    // (linux doesn't care...)
    if (offset < size)
    {
        void* nbuf = io.Mmap(
            offset, std::min<FileMemSize>(MMAP_CHUNK, size-offset), true);
        io.Munmap(buf, buf_size);
        buf = static_cast<Byte*>(nbuf);
        buf_put = len;
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
        std::cerr << "Mayday: ~SimpleSink";
        PrintException(std::cerr);
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

void SimpleSink::Write_(const Byte* data, FileMemSize len)
{
    BOOST_ASSERT(buf_size == MEM_CHUNK);
    BOOST_ASSERT(buf_put == MEM_CHUNK);
    io.Write(buf, MEM_CHUNK);
    offset += MEM_CHUNK;

    if (len >= MEM_CHUNK)
    {
        io.Write(data, len);
        offset += len;
        buf_put = 0;
    }
    else
    {
        memcpy(buf, data, len);
        buf_put = len;
    }
}

void SimpleSink::Pad_(FileMemSize len)
{
    BOOST_ASSERT(buf_size == MEM_CHUNK);
    BOOST_ASSERT(buf_put == MEM_CHUNK);
    io.Write(buf, MEM_CHUNK);
    offset += MEM_CHUNK;

    // assume we're not seekable (I don't care about not mmap-able but seekable
    // files)
    if (len >= MEM_CHUNK)
    {
        memset(buf, 0, MEM_CHUNK);
        size_t i;
        for (i = MEM_CHUNK; i < len; i += MEM_CHUNK)
            io.Write(buf, MEM_CHUNK);
        offset += i - MEM_CHUNK;
    }
    else
        memset(buf, 0, len);
    buf_put = len % MEM_CHUNK;
}

std::unique_ptr<Sink> Sink::ToFile(
    fs::path fname, FilePosition size, bool try_mmap)
{
    return AddInfo(
        [&]() -> std::unique_ptr<Sink>
        {
            LowIo io{fname.c_str(), true};
            if (!try_mmap)
                return std::make_unique<SimpleSink>(std::move(io), size);

            try { return std::make_unique<MmapSink>(std::move(io), size); }
            catch (const std::system_error& e)
            {
                std::cerr << "Mmmap failed: ";
                PrintException(std::cerr);
                return std::make_unique<SimpleSink>(std::move(io), size);
            }
        },
        [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; });
}
