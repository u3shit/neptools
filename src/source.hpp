#ifndef UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#define UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#pragma once

#include "dumpable.hpp"
#include "endian.hpp"
#include "low_io.hpp"

#include <libshit/check.hpp>
#include <libshit/lua/value_object.hpp>
#include <libshit/meta_utils.hpp>
#include <libshit/nonowning_string.hpp>
#include <libshit/not_null.hpp>
#include <libshit/shared_ptr.hpp>

#include <array>
#include <cstdint>
#include <boost/filesystem/path.hpp>

namespace Neptools
{

  LIBSHIT_GEN_EXCEPTION_TYPE(SourceOverflow, std::logic_error);

  /// A fixed size, read-only, seekable data source (or something that emulates
  /// it)
  class LIBSHIT_LUAGEN(const=false) Source final
    : public Libshit::Lua::ValueObject
  {
    LIBSHIT_LUA_CLASS;
  public:
    struct BufEntry
    {
      const Byte* ptr = nullptr;
      FilePosition offset = -1;
      FileMemSize size = 0;
    };

    Source(Source s, FilePosition offset, FilePosition size) noexcept
      : Source{std::move(s)} { Slice(offset, size); get = 0; }

    static Source FromFile(const boost::filesystem::path& fname);
    static Source FromMemory(std::string data)
    { return FromMemory("", std::move(data)); }
    static Source FromMemory(
      const boost::filesystem::path& fname, std::string data);

    template <typename Checker = Libshit::Check::Assert>
      void Slice(FilePosition offset, FilePosition size) noexcept
    {
      LIBSHIT_CHECK(SourceOverflow, offset <= this->size &&
                    offset + size <= this->size, "Slice: invalid sizes");
      this->offset += offset;
      this->get -= offset;
      this->size = size;
    }

    FilePosition GetOffset() const noexcept { return offset; }
    FilePosition GetOrigSize() const noexcept { return p->size; }
    const boost::filesystem::path& GetFileName() const noexcept
    { return p->file_name; }

    FilePosition GetSize() const noexcept { return size; }

    template <typename Checker = Libshit::Check::Assert>
    void Seek(FilePosition pos) noexcept
    {
      LIBSHIT_CHECK(SourceOverflow, pos <= size, "Seek past end of source");
      get = pos;
    }
    FilePosition Tell() const noexcept { return get; }
    FilePosition GetRemainingSize() const noexcept { return size - get; }
    bool Eof() const noexcept { return get == size; }

    void CheckSize(FilePosition size) const
    {
      if (p->size < size)
        LIBSHIT_THROW(Libshit::DecodeError, "Premature end of data",
                      "Used source", *this);
    }
    void CheckRemainingSize(FilePosition size) const { CheckSize(get + size); }

    template <typename Checker = Libshit::Check::Assert, typename T>
    LIBSHIT_NOLUA void ReadGen(T& x)
    { Read<Checker>(reinterpret_cast<Byte*>(&x), Libshit::EmptySizeof<T>); }

    template <typename T, typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA T ReadGen() { T ret; ReadGen<Checker>(ret); return ret; }


    template <typename Checker = Libshit::Check::Assert, typename T>
    LIBSHIT_NOLUA void PreadGen(FilePosition offs, T& x) const
    { Pread<Checker>(offs, reinterpret_cast<Byte*>(&x), Libshit::EmptySizeof<T>); }

    template <typename T, typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA T PreadGen(FilePosition offs) const
    { T ret; PreadGen<Checker>(offs, ret); return ret; }


    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA void Read(Byte* buf, FileMemSize len)
    { Pread<Checker>(get, buf, len); get += len; }
    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA void Read(char* buf, FileMemSize len)
    { Pread<Checker>(get, buf, len); get += len; }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA void Pread(
      FilePosition offs, Byte* buf, FileMemSize len) const
    {
      LIBSHIT_ADD_INFOS(
        LIBSHIT_CHECK(SourceOverflow, offs <= size && offs+len <= size,
                      "Source overflow");
        Pread_(offs, buf, len),
        "Used source", *this, "Read offset", offs, "Read size", len);
    }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA
    void Pread(FilePosition offs, char* buf, FileMemSize len) const
    { Pread<Checker>(offs, reinterpret_cast<Byte*>(buf), len); }

    // helper
#define NEPTOOLS_GEN_HLP2(bits, Camel, snake)                             \
    template <typename Checker = Libshit::Check::Assert>                  \
    std::uint##bits##_t Read##Camel##Uint##bits()                         \
    {                                                                     \
      return boost::endian::snake##_to_native(                            \
        ReadGen<std::uint##bits##_t, Checker>());                         \
    }                                                                     \
    template <typename Checker = Libshit::Check::Assert>                  \
    std::uint##bits##_t Pread##Camel##Uint##bits(FilePosition offs) const \
    {                                                                     \
      return boost::endian::snake##_to_native(                            \
        PreadGen<std::uint##bits##_t, Checker>(offs));                    \
    }
#define NEPTOOLS_GEN_HLP(bits)                                           \
    template <typename Checker = Libshit::Check::Assert>                 \
    std::uint##bits##_t ReadUint##bits(Endian e)                         \
    {                                                                    \
      return ToNativeCopy(ReadGen<std::uint##bits##_t, Checker>(), e);   \
    }                                                                    \
    template <typename Checker = Libshit::Check::Assert>                 \
    std::uint##bits##_t PreadUint##bits(FilePosition offs, Endian e)     \
    {                                                                    \
      return ToNativeCopy(                                               \
        PreadGen<std::uint##bits##_t, Checker>(offs), e);                \
    }                                                                    \
    NEPTOOLS_GEN_HLP2(bits, Little, little)                              \
    NEPTOOLS_GEN_HLP2(bits, Big, big)

    // 8-bit values have no endian, but have these functions for consistency
    NEPTOOLS_GEN_HLP(8)
    NEPTOOLS_GEN_HLP(16)
    NEPTOOLS_GEN_HLP(32)
    NEPTOOLS_GEN_HLP(64)
#undef NEPTOOLS_GEN_HLP

      std::string ReadCString()
    {
      auto ret = PreadCString(Tell());
      Seek(Tell() + ret.size() + 1);
      return ret;
    }
    std::string PreadCString(FilePosition offs) const;

    struct Provider : public Libshit::RefCounted
    {
      Provider(boost::filesystem::path file_name, FilePosition size)
        : file_name{std::move(file_name)}, size{size} {}
      Provider(const Provider&) = delete;
      void operator=(const Provider&) = delete;
      virtual ~Provider() = default;

      virtual void Pread(FilePosition offs, Byte* buf, FileMemSize len) = 0;

      void LruPush(const Byte* ptr, FilePosition offset, FileMemSize size);
      bool LruGet(FilePosition offs);

      std::array<BufEntry, 4> lru;
      boost::filesystem::path file_name;
      FilePosition size;
    };
    LIBSHIT_NOLUA
      Source(Libshit::NotNull<Libshit::SmartPtr<Provider>> p, FilePosition size)
      : size{size}, p{std::move(p)} {}

    void Dump(Sink& sink) const;
    LIBSHIT_NOLUA void Dump(Sink&& sink) const { Dump(sink); }
    LIBSHIT_NOLUA void Inspect(std::ostream& os) const;
    LIBSHIT_NOLUA void Inspect(std::ostream&& os) const { Inspect(os); }
    std::string Inspect() const;

    LIBSHIT_NOLUA Libshit::StringView GetChunk(FilePosition offs) const;

  private:
    // offset: in original file!
    BufEntry GetTemporaryEntry(FilePosition offs) const;

    void Pread_(FilePosition offs, Byte* buf, FileMemSize len) const;
    static Source FromFile_(const boost::filesystem::path& fname);

    FilePosition offset = 0, size, get = 0;

    Libshit::NotNull<Libshit::SmartPtr<Provider>> p;
  };

  inline std::ostream& operator<<(std::ostream& os, const Source s)
  { s.Inspect(os); return os; }

  std::string to_string(const Source& s);

  class DumpableSource final : public Dumpable
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    LIBSHIT_NOLUA
    DumpableSource(Source&& s) noexcept : src{std::move(s)} {}
    DumpableSource(const Source& s, FilePosition offset, FilePosition size) noexcept
      : src{s, offset, size} {}
    DumpableSource(const Source& s) noexcept : src{s} {} // NOLINT

    void Fixup() override {}

    FilePosition GetSize() const override { return src.GetSize(); }
    Source GetSource() const noexcept { return src; }
  private:
    Source src;
    void Dump_(Sink& sink) const override { src.Dump(sink); }
    void Inspect_(std::ostream& os, unsigned) const override;
  };

#define ADD_SOURCE(expr, ...) \
  LIBSHIT_ADD_INFOS(expr, "Used source", __VA_ARGS__)

  struct QuotedSource { Source src; };
  inline std::ostream& operator<<(std::ostream& os, QuotedSource q)
  { DumpBytes(os, q.src); return os; }
  inline QuotedSource Quoted(Source src) { return {src}; }

}
#endif
