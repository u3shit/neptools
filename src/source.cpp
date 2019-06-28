#include "source.hpp"
#include "sink.hpp"

#include <libshit/char_utils.hpp>
#include <libshit/except.hpp>
#include <libshit/lua/function_call.hpp>
#include <libshit/platform.hpp>

#include <fstream>
#include <iostream>

#include <unistd.h>

#include <libshit/doctest.hpp>

#define LIBSHIT_LOG_NAME "source"
#include <libshit/logger_helper.hpp>

namespace Neptools
{
  TEST_SUITE_BEGIN("Neptools::Source");

  namespace
  {

    template <typename T>
    struct UnixLike : public Source::Provider
    {
      UnixLike(LowIo io, boost::filesystem::path file_name, FilePosition size)
        : Source::Provider{std::move(file_name), size}, io{std::move(io)} {}
      ~UnixLike();

      void Pread(FilePosition offs, Byte* buf, FileMemSize len) override;
      void EnsureChunk(FilePosition i);

      LowIo io;
    };

    struct MmapProvider final : public UnixLike<MmapProvider>
    {
      MmapProvider(LowIo&& fd, boost::filesystem::path file_name,
                   FilePosition size);

      static FileMemSize CHUNK_SIZE;
      void* ReadChunk(FilePosition offs, FileMemSize size);
      void DeleteChunk(size_t i);
    };

    struct UnixProvider final : public UnixLike<UnixProvider>
    {
      //using UnixLike::UnixLike;
      // workaround clang bug...
      UnixProvider(LowIo&& io, boost::filesystem::path file_name,
                   FilePosition size)
        : UnixLike{std::move(io), std::move(file_name), size} {}

      static FileMemSize CHUNK_SIZE;
      void* ReadChunk(FilePosition offs, FileMemSize size);
      void DeleteChunk(size_t i);
    };

    struct StringProvider final : public Source::Provider
    {
      StringProvider(boost::filesystem::path file_name, std::string str)
        : Source::Provider(std::move(file_name), str.size()),
        str{std::move(str)}
      {
        LruPush(reinterpret_cast<const Byte*>(this->str.data()),
                0, this->str.size());
      }

      void Pread(FilePosition, Byte*, FileMemSize) override
      { LIBSHIT_UNREACHABLE("StringProvider Pread"); }

      std::string str;
    };

    struct UniquePtrProvider final : public Source::Provider
    {
      UniquePtrProvider(boost::filesystem::path file_name,
                        std::unique_ptr<char[]> data, std::size_t len)
        : Source::Provider(std::move(file_name), len),
          data{Libshit::Move(data)}
      { LruPush(reinterpret_cast<const Byte*>(this->data.get()), 0, len); }

      void Pread(FilePosition, Byte*, FileMemSize) override
      { LIBSHIT_UNREACHABLE("UniquePtrProvider Pread"); }

      std::unique_ptr<char[]> data;
    };

  }


  Source Source::FromFile(const boost::filesystem::path& fname)
  {
    LIBSHIT_ADD_INFOS(return FromFile_(fname), "File name", fname.string());
  }

  Source Source::FromFile_(const boost::filesystem::path& fname)
  {
    LowIo io{fname.c_str(), false};

    FilePosition size = io.GetSize();

    Libshit::SmartPtr<Provider> p;
    try { p = Libshit::MakeSmart<MmapProvider>(std::move(io), fname, size); }
    catch (const std::system_error& e)
    {
      WARN << "Mmap failed, falling back to normal reading: "
           << Libshit::ExceptionToString() << std::endl;
      p = Libshit::MakeSmart<UnixProvider>(std::move(io), fname, size);
    }
    return Libshit::MakeNotNull(Libshit::Move(p));
  }

  Source Source::FromFd(
    boost::filesystem::path fname, LowIo::FdType fd, bool owning)
  {
    LowIo io{fd, owning};
    auto size = io.GetSize();
    return {Libshit::MakeSmart<UnixProvider>(
        Libshit::Move(io), Libshit::Move(fname), size)};
  }

  Source Source::FromMemory(boost::filesystem::path fname, std::string str)
  {
    return {Libshit::MakeSmart<StringProvider>(
        Libshit::Move(fname), Libshit::Move(str))};
  }

  Source Source::FromMemory(boost::filesystem::path fname,
                            std::unique_ptr<char[]> data, std::size_t len)
  {
    return {Libshit::MakeSmart<UniquePtrProvider>(
        Libshit::Move(fname), Libshit::Move(data), len)};
  }


  void Source::Pread_(FilePosition offs, Byte* buf, FileMemSize len) const
  {
    offs += offset;
    while (len)
    {
      if (p->LruGet(offs))
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
  }

  std::string Source::PreadCString(FilePosition offs) const
  {
    std::string ret;
    Libshit::StringView e;
    size_t len;
    do
    {
      e = GetChunk(offs);
      len = strnlen(e.data(), e.size());
      ret.append(e.data(), len);
      offs += e.size();
    } while (len == e.size());
    return ret;
  }

  Source::BufEntry Source::GetTemporaryEntry(FilePosition offs) const
  {
    if (p->LruGet(offs)) return p->lru[0];
    p->Pread(offs, nullptr, 0);
    LIBSHIT_ASSERT(p->lru[0].offset <= offs &&
                   p->lru[0].offset + p->lru[0].size > offs);
    return p->lru[0];
  }

  Libshit::StringView Source::GetChunk(FilePosition offs) const
  {
    LIBSHIT_ASSERT(offs < size);
    auto e = GetTemporaryEntry(offs + offset);
    auto eoffs = offs + offset - e.offset;
    auto size = std::min(e.size - eoffs, GetSize() - offs);
    return { e.ptr + eoffs, size };
  }


  void Source::Provider::LruPush(
    const Byte* ptr, FilePosition offset, FileMemSize size)
  {
    memmove(&lru[1], &lru[0], sizeof(BufEntry)*(lru.size()-1));
    lru[0].ptr = ptr;
    lru[0].offset = offset;
    lru[0].size = size;
  }

  bool Source::Provider::LruGet(FilePosition offs)
  {
    for (size_t i = 0; i < lru.size(); ++i)
    {
      auto x = lru[i];
      if (x.offset <= offs && x.offset + x.size > offs)
      {
        memmove(&lru[1], &lru[0], sizeof(BufEntry)*i);
        lru[0] = x;
        return true;
      }
    }
    return false;
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
    LIBSHIT_ASSERT(io.fd != LowIo::INVALID_FD);
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
    if (LruGet(offs)) return;

    auto size = std::min(CHUNK_SIZE, this->size-ch_offs);
    auto x = static_cast<T*>(this)->ReadChunk(ch_offs, size);
    static_cast<T*>(this)->DeleteChunk(lru.size()-1);
    LruPush(static_cast<Byte*>(x), ch_offs, size);
  }

  FileMemSize MmapProvider::CHUNK_SIZE = LowIo::MMAP_CHUNK;
  MmapProvider::MmapProvider(
    LowIo&& io, boost::filesystem::path file_name, FilePosition size)
    : UnixLike{{}, std::move(file_name), size}
  {
    size_t to_map = size < LowIo::MMAP_LIMIT ? size : LowIo::MMAP_CHUNK;

    io.PrepareMmap(false);
    void* ptr = io.Mmap(0, to_map, false);
#if !LIBSHIT_OS_IS_WINDOWS
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
      io.Munmap(const_cast<Byte*>(lru[i].ptr), lru[i].size);
  }

  FilePosition UnixProvider::CHUNK_SIZE = LowIo::MEM_CHUNK;

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

  void Source::Inspect(std::ostream& os) const
  {
    os << "neptools.source.from_memory("
       << Libshit::Quoted(GetFileName().string()) << ", "
       << Quoted(*this)  << ")";
  }

  std::string Source::Inspect() const
  {
    std::stringstream ss;
    Inspect(ss);
    return ss.str();
  }

  void Source::Dump(Sink& sink) const
  {
    FilePosition offset = 0;
    auto size = GetSize();
    while (offset < size)
    {
      auto chunk = GetChunk(offset);
      sink.Write(chunk);
      offset += chunk.size();
    }
  }

  void DumpableSource::Inspect_(std::ostream& os, unsigned) const
  {
    os << "neptools.dumpable_source(";
    src.Inspect(os);
    os << ')';
  }

  std::string to_string(const Source& src)
  {
    std::stringstream ss;
    ss << src.GetFileName() << ", pos: " << src.Tell();
    return ss.str();
  }

#if LIBSHIT_WITH_LUA

  LIBSHIT_LUAGEN(name="read")
  static Libshit::Lua::RetNum LuaRead(
    Libshit::Lua::StateRef vm, Source& src, FileMemSize len)
  {
    std::unique_ptr<char[]> ptr{new char[len]};
    src.Read<Libshit::Check::Throw>(ptr.get(), len);
    lua_pushlstring(vm, ptr.get(), len);
    return {1};
  }

  LIBSHIT_LUAGEN(name="pread")
  static Libshit::Lua::RetNum LuaPread(
    Libshit::Lua::StateRef vm, Source& src, FilePosition offs, FileMemSize len)
  {
    std::unique_ptr<char[]> ptr{new char[len]};
    src.Pread<Libshit::Check::Throw>(offs, ptr.get(), len);
    lua_pushlstring(vm, ptr.get(), len);
    return {1};
  }

#endif

  TEST_CASE("small source")
  {
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    std::ofstream os{"tmp", std::ios_base::binary};
    os.write(buf, 16);
    os.close();

    auto src = Source::FromFile("tmp");
    char buf2[16];
    src.ReadGen(buf2);
    REQUIRE(memcmp(buf, buf2, 16) == 0);
    CHECK(src.Inspect() ==
          R"(neptools.source.from_memory("tmp", "\x00\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0e\x0f"))");
  }
  TEST_SUITE_END();
}

#include "source.binding.hpp"
