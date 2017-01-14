#ifndef UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#define UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#pragma once

#include <array>
#include <boost/endian/arithmetic.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/filesystem/path.hpp>
#include "check.hpp"
#include "dumpable.hpp"
#include "low_io.hpp"
#include "not_null.hpp"
#include "shared_ptr.hpp"
#include "lua/value_object.hpp"

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(SourceOverflow, std::logic_error);

class Source;
using UsedSource = boost::error_info<struct UsedSourceTag, Source>;
using ReadOffset = boost::error_info<struct ReadOffsetTag, FilePosition>;
using ReadSize = boost::error_info<struct ReadOffsetTag, FileMemSize>;
std::string to_string(const UsedSource& src);

/// A fixed size, read-only, seekable data source (or something that emulates it)
class Source final : public Lua::ValueObject
{
    NEPTOOLS_LUA_CLASS;
public:
    struct BufEntry
    {
        Byte* ptr = nullptr;
        FilePosition offset = -1;
        FileMemSize size = 0;
    };

    Source(const Source& s, FilePosition offset, FilePosition size) noexcept
        : Source{s} { Slice(offset, size); get = 0; }

    static Source FromFile(boost::filesystem::path fname);

    template <typename Checker = Check::Assert>
    void Slice(FilePosition offset, FilePosition size) noexcept
    {
        NEPTOOLS_CHECK(SourceOverflow, offset <= this->size &&
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

    template <typename Checker = Check::Assert>
    void Seek(FilePosition pos) noexcept
    {
        NEPTOOLS_CHECK(SourceOverflow, pos <= size, "Seek past end of source");
        get = pos;
    }
    FilePosition Tell() const noexcept { return get; }
    FilePosition GetRemainingSize() const noexcept { return p->size - get; }
    bool Eof() const noexcept { return get == size; }

    void CheckSize(FilePosition size) const
    {
        if (p->size < size)
            NEPTOOLS_THROW(
                DecodeError{"Premature end of data"} <<
                UsedSource(*this));
    }
    void CheckRemainingSize(FilePosition size) const { CheckSize(get + size); }

    template <typename Checker = Check::Assert, typename T>
    NEPTOOLS_NOLUA void ReadGen(T& x)
    { Read<Checker>(reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }

    template <typename T, typename Checker = Check::Assert>
    NEPTOOLS_NOLUA T ReadGen() { T ret; ReadGen<Checker>(ret); return ret; }


    template <typename Checker = Check::Assert, typename T>
    NEPTOOLS_NOLUA void PreadGen(FilePosition offs, T& x) const
    { Pread<Checker>(offs, reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }

    template <typename T, typename Checker = Check::Assert>
    NEPTOOLS_NOLUA T PreadGen(FilePosition offs) const
    { T ret; PreadGen<Checker>(offs, ret); return ret; }


    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA void Read(Byte* buf, FileMemSize len)
    { Pread<Checker>(get, buf, len); get += len; }
    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA void Read(char* buf, FileMemSize len)
    { Pread<Checker>(get, buf, len); get += len; }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA
    void Pread(FilePosition offs, Byte* buf, FileMemSize len) const
    {
        AddInfo([&]
        {
            NEPTOOLS_CHECK(SourceOverflow, offs <= size && offs+len <= size,
                           "Source overflow");
            Pread_(offs, buf, len);
        },
        [=] (auto& e)
        {
            e << UsedSource{*this} << ReadOffset{offs} << ReadSize{len};
        });
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA
    void Pread(FilePosition offs, char* buf, FileMemSize len) const
    { Pread<Checker>(offs, reinterpret_cast<Byte*>(buf), len); }

    // helper
#define NEPTOOLS_GEN_HLP(bits)                                              \
    template <typename Checker = Check::Assert>                             \
    uint##bits##_t ReadLittleUint##bits()                                   \
    { return ReadGen<boost::endian::little_uint##bits##_t, Checker>(); }    \
    template <typename Checker = Check::Assert>                             \
    uint##bits##_t PreadLittleUint##bits(FilePosition offs) const           \
    { return PreadGen<boost::endian::little_uint##bits##_t, Checker>(offs); }

    NEPTOOLS_GEN_HLP(8)
    NEPTOOLS_GEN_HLP(16)
    NEPTOOLS_GEN_HLP(32)
    NEPTOOLS_GEN_HLP(64)
#undef NEPTOOLS_GEN_HLP

    static constexpr size_t GET_C_BUF_SIZE = 16;
    std::string ReadCString()
    {
        std::string str;
        char buf[GET_C_BUF_SIZE];
        while (true)
        {
            auto rem = GetRemainingSize();
            if (rem == 0)
                NEPTOOLS_THROW(DecodeError{"Unterminated C-style string"} <<
                               UsedSource{*this});

            auto rd = std::min<FilePosition>(GET_C_BUF_SIZE, rem);
            Read(buf, rd);
            auto len = strnlen(buf, rd);
            str.append(buf, len);
            if (len < rd)
            {
                Seek(Tell() - (rd-len+1));
                break;
            }
        }
        return str;
    }

    std::string PreadCString(FilePosition offs) const
    {
        std::string str;
        char buf[GET_C_BUF_SIZE];
        while (true)
        {
            auto rem = GetSize() - offs;
            if (rem == 0)
                NEPTOOLS_THROW(DecodeError{"Unterminated C-style string"} <<
                               UsedSource{*this});

            auto rd = std::min<FilePosition>(GET_C_BUF_SIZE, rem);
            Pread(offs, buf, rd);
            auto len = strnlen(buf, rd);
            str.append(buf, len);
            if (len < rd) break;
            offs += rd;
        }
        return str;
    }


    struct Provider : public RefCounted
    {
        Provider(boost::filesystem::path file_name, FilePosition size)
            : file_name{std::move(file_name)}, size{size} {}
        Provider(const Provider&) = delete;
        void operator=(const Provider&) = delete;
        virtual ~Provider() = default;

        virtual void Pread(FilePosition offs, Byte* buf, FileMemSize len) = 0;

        void LruPush(Byte* ptr, FilePosition offset, FileMemSize size);
        bool LruGet(FilePosition offs);

        std::array<BufEntry, 4> lru;
        boost::filesystem::path file_name;
        FilePosition size;
    };
    NEPTOOLS_NOLUA
    Source(NotNull<SmartPtr<Provider>> p, FilePosition size)
        : size{size}, p{std::move(p)} {}

    void Dump(Sink& sink) const;
    NEPTOOLS_NOLUA void Dump(Sink&& sink) const { Dump(sink); }
    NEPTOOLS_NOLUA void Inspect(std::ostream& os) const;
    NEPTOOLS_NOLUA void Inspect(std::ostream&& os) const { Inspect(os); }
    std::string Inspect() const;

private:
    // offset: in original file!
    BufEntry GetTemporaryEntry(FilePosition offs) const;

    void Pread_(FilePosition offs, Byte* buf, FileMemSize len) const;
    static Source FromFile_(boost::filesystem::path fname);

    FilePosition offset = 0, size, get = 0;

    NotNull<SmartPtr<Provider>> p;
};

class DumpableSource final : public Dumpable
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    DumpableSource(const Source& s) noexcept : src{s} {}
    NEPTOOLS_NOLUA
    DumpableSource(Source&& s) noexcept : src{std::move(s)} {}
    DumpableSource(const Source& s, FilePosition offset, FilePosition size) noexcept
        : src{s, offset, size} {}

    void Fixup() override {}

    FilePosition GetSize() const override { return src.GetSize(); }
    Source GetSource() const noexcept { return src; }
private:
    Source src;
    void Dump_(Sink& sink) const override { src.Dump(sink); }
    void Inspect_(std::ostream& os) const override { src.Inspect(os); }
};

#define ADD_SOURCE(src)                                         \
    [&](auto& add_source_e)                                     \
    {                                                           \
        if (!::boost::get_error_info<UsedSource>(add_source_e)) \
            add_source_e << UsedSource{src};                    \
    }

}
#endif
