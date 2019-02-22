#include "sink.hpp"
#include "low_io.hpp"

#include <libshit/except.hpp>
#include <libshit/lua/boost_endian_traits.hpp>

#include <iostream>
#include <fstream>

#include <libshit/doctest.hpp>

#define LIBSHIT_LOG_NAME "sink"
#include <libshit/logger_helper.hpp>

namespace Neptools
{
  TEST_SUITE_BEGIN("Neptools::Sink");
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
      [&](auto& e) { Libshit::AddInfos(e, "File name", fname.string()); });
  }

  Libshit::NotNull<Libshit::RefCountedPtr<Sink>> Sink::ToStdOut()
  {
    return Libshit::MakeRefCounted<SimpleSink>(LowIo::OpenStdOut(), -1);
  }

#define TRY_MMAP                           \
  bool try_mmap;                           \
  SUBCASE("mmap") { try_mmap = false; }    \
  SUBCASE("no mmap") { try_mmap = true; }  \
  CAPTURE(try_mmap)

  TEST_CASE("small simple write")
  {
    TRY_MMAP;
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    {
      auto sink = Sink::ToFile("tmp", 16, try_mmap);
      REQUIRE(sink->Tell() == 0);
      sink->WriteGen(buf);
      REQUIRE(sink->Tell() == 16);
    }

    char buf2[16];
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("many small writes")
  {
    TRY_MMAP;
    int buf[6] = {0,77,-123,98,77,-1};
    static_assert(sizeof(buf) == 24);

    static constexpr FilePosition SIZE = 2*1024*1024 / 24 * 24;
    {
      auto sink = Sink::ToFile("tmp", SIZE, try_mmap);
      for (FilePosition i = 0; i < SIZE; i += 24)
      {
        buf[0] = i/24;
        sink->WriteGen(buf);
      }
      REQUIRE(sink->Tell() == SIZE);
    }

    std::unique_ptr<char[]> buf_exp{new char[SIZE]};
    for (FilePosition i = 0; i < SIZE; i += 24)
    {
      buf[0] = i/24;
      memcpy(buf_exp.get()+i, buf, 24);
    }

    std::unique_ptr<char[]> buf_act{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf_act.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf_exp.get(), buf_act.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("big write")
  {
    TRY_MMAP;
    static constexpr FilePosition SIZE = 2*1024*1024;
    std::unique_ptr<Byte[]> buf{new Byte[SIZE]};
    for (size_t i = 0; i < SIZE; ++i)
      buf[i] = i;

    {
      auto sink = Sink::ToFile("tmp", SIZE, try_mmap);
      REQUIRE(sink->Tell() == 0);
      sink->Write({buf.get(), SIZE});
      REQUIRE(sink->Tell() == SIZE);
    }

    std::unique_ptr<char[]> buf2{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf.get(), buf2.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("small pad")
  {
    TRY_MMAP;
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    {
      auto sink = Sink::ToFile("tmp", 16*3, try_mmap);
      REQUIRE(sink->Tell() == 0);
      sink->WriteGen(buf);
      REQUIRE(sink->Tell() == 16);
      sink->Pad(16);
      REQUIRE(sink->Tell() == 2*16);
      sink->WriteGen(buf);
      REQUIRE(sink->Tell() == 3*16);
    }

    char buf2[16];
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(std::count(buf2, buf2+16, 0) == 16);

    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("many small pad")
  {
    TRY_MMAP;
    char buf[10] = {4,5,6,7,8,9,10,11,12,13};
    static constexpr FilePosition SIZE = 2*1024*1024 / 24 * 24;
    {
      auto sink = Sink::ToFile("tmp", SIZE, try_mmap);

      for (FilePosition i = 0; i < SIZE; i += 24)
      {
        sink->WriteGen(buf);
        sink->Pad(14);
      }
      REQUIRE(sink->Tell() == SIZE);
    }

    std::unique_ptr<char[]> buf_exp{new char[SIZE]};
    for (FilePosition i = 0; i < SIZE; i += 24)
    {
      memcpy(buf_exp.get()+i, buf, 10);
      memset(buf_exp.get()+i+10, 0, 14);
    }

    std::unique_ptr<char[]> buf_act{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf_act.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf_exp.get(), buf_act.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("large pad")
  {
    TRY_MMAP;
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    static constexpr FilePosition ZERO_SIZE = 2*1024*1024;
    {
      auto sink = Sink::ToFile("tmp", 16*2 + ZERO_SIZE, try_mmap);
      REQUIRE(sink->Tell() == 0);
      sink->WriteGen(buf);
      REQUIRE(sink->Tell() == 16);
      sink->Pad(ZERO_SIZE);
      REQUIRE(sink->Tell() == 16+ZERO_SIZE);
      sink->WriteGen(buf);
      REQUIRE(sink->Tell() == 2*16+ZERO_SIZE);
    }

    std::unique_ptr<char[]> buf2{new char[ZERO_SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2.get(), 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2.get(), 16) == 0);

    is.read(buf2.get(), ZERO_SIZE);
    REQUIRE(is.good());
    REQUIRE(std::count(buf2.get(), buf2.get()+ZERO_SIZE, 0) == ZERO_SIZE);

    is.read(buf2.get(), 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2.get(), 16) == 0);

    is.get();
    REQUIRE(is.eof());
  }

  TEST_CASE("sink helpers")
  {
    TRY_MMAP;
    {
      auto sink = Sink::ToFile("tmp", 15, try_mmap);
      sink->WriteLittleUint8(247);
      sink->WriteLittleUint16(1234);
      sink->WriteLittleUint32(98765);
      sink->WriteCString("asd");
      sink->WriteCString(std::string{"def"});
    }

    Byte exp[15] = { 247, 0xd2, 0x04, 0xcd, 0x81, 0x01, 0x00,
                     'a', 's', 'd', 0, 'd', 'e', 'f', 0 };
    char act[15];

    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(act, 15);
    REQUIRE(is.good());
    REQUIRE(memcmp(exp, act, 15) == 0);

    is.get();
    REQUIRE(is.eof());
  }


  void MemorySink::Write_(Libshit::StringView)
  { LIBSHIT_UNREACHABLE("MemorySink::Write_ called"); }
  void MemorySink::Pad_(FileMemSize)
  { LIBSHIT_UNREACHABLE("MemorySink::Pad_ called"); }

  TEST_CASE("memory one write")
  {
    Byte buf[16] = {15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
    Byte buf2[16];
    {
      MemorySink sink{buf2, 16};
      sink.WriteGen(buf);
    }

    REQUIRE(memcmp(buf, buf2, 16) == 0);
  }

  TEST_CASE("memory multiple writes")
  {
    Byte buf[8] = {42,43,44,45,46,47,48,49};
    Byte buf_out[32];
    Byte buf_exp[32];
    {
      MemorySink sink{buf_out, 32};
      for (size_t i = 0; i < 32; i+=8)
      {
        memcpy(buf_exp+i, buf, 8);
        sink.WriteGen(buf);
        buf[0] = i;
      }
    }

    REQUIRE(memcmp(buf_out, buf_exp, 32) == 0);
  }

  TEST_CASE("memory pad")
  {
    Byte buf[8] = {77,78,79,80,81,82,83,84};
    Byte buf_out[32];
    Byte buf_exp[32];
    {
      MemorySink sink{buf_out, 32};
      memcpy(buf_exp, buf, 8);
      sink.WriteGen(buf);

      memset(buf_exp+8, 0, 16);
      sink.Pad(16);

      memcpy(buf_exp+24, buf, 8);
      sink.WriteGen(buf);
    }

    REQUIRE(memcmp(buf_out, buf_exp, 32) == 0);
  }

  TEST_CASE("memory alloc by itself")
  {
    char buf_exp[4] = { 0x78, 0x56, 0x34, 0x12 };
    MemorySink sink{4};

    sink.WriteLittleUint32(0x12345678);
    auto buf = sink.Release();

    REQUIRE(memcmp(buf.get(), buf_exp, 4) == 0);
  }


#if LIBSHIT_WITH_LUA
  LIBSHIT_LUAGEN(name="new", class="Neptools::MemorySink")
  static Libshit::NotNull<Libshit::SmartPtr<MemorySink>>
  MemorySinkFromLua(Libshit::StringView view)
  {
    std::unique_ptr<Byte[]> buf{new Byte[view.length()]};
    memcpy(buf.get(), view.data(), view.length());
    return Libshit::MakeSmart<MemorySink>(std::move(buf), view.length());
  }
#endif

  TEST_SUITE_END();
}

#include "sink.binding.hpp"
