#ifndef UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#define UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#pragma once

#include <array>
#include <boost/endian/arithmetic.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/filesystem/path.hpp>
#include "dumpable.hpp"
#include "low_io.hpp"

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(SourceUnderflow, std::logic_error);

class Source;
using UsedSource = boost::error_info<struct UsedSourceTag, Source>;
using ReadOffset = boost::error_info<struct ReadOffsetTag, FilePosition>;
using ReadSize = boost::error_info<struct ReadOffsetTag, FileMemSize>;
std::string to_string(const UsedSource& src);

/// A fixed size, read-only, seekable data source (or something that emulates it)
class Source
{
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

    void Slice(FilePosition offset, FilePosition size) noexcept
    {
        NEPTOOLS_ASSERT_MSG(offset <= this->size &&
                            offset + size <= this->size, "source overflow");
        this->offset += offset;
        this->get -= offset;
        this->size = size;
    }

    FilePosition GetOffset() const noexcept { return offset; }
    FilePosition GetOrigSize() const noexcept { return p->size; }
    const boost::filesystem::path& GetFileName() const noexcept
    { return p->file_name; }

    FilePosition GetSize() const noexcept { return size; }

    void Seek(FilePosition pos) noexcept
    {
        NEPTOOLS_ASSERT_MSG(pos <= size, "source overflow");
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

    template <typename T>
    void ReadGen(T& x) { Read(reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }
    template <typename T>
    T ReadGen() { T ret; ReadGen(ret); return ret; }
    template <typename T>
    void CheckedReadGen(T& x)
    { CheckedRead(reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }
    template <typename T>
    T CheckedReadGen() { T ret; CheckedReadGen(ret); return ret; }

    template <typename T>
    void PreadGen(FilePosition offs, T& x) const
    { Pread(offs, reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }
    template <typename T>
    T PreadGen(FilePosition offs) const
    { T ret; PreadGen(offs, ret); return ret; }
    template <typename T>
    void CheckedPreadGen(FilePosition offs, T& x) const
    { CheckedPread(offs, reinterpret_cast<Byte*>(&x), EmptySizeof<T>); }
    template <typename T>
    T CheckedPreadGen(FilePosition offs) const
    { T ret; CheckedPreadGen(offs, ret); return ret; }


    void Read(Byte* buf, FileMemSize len) { Pread(get, buf, len); get += len; }
    void Read(char* buf, FileMemSize len) { Pread(get, buf, len); get += len; }
    void Pread(FilePosition offs, Byte* buf, FileMemSize len) const;
    void Pread(FilePosition offs, char* buf, FileMemSize len) const
    { Pread(offs, reinterpret_cast<Byte*>(buf), len); }

    void CheckedRead(Byte* buf, FileMemSize len) { CheckedPread(get, buf, len); get += len; }
    void CheckedRead(char* buf, FileMemSize len) { CheckedPread(get, buf, len); get += len; }
    void CheckedPread(FilePosition offs, Byte* buf, FileMemSize len) const
    {
        if (offs > size || offs+len > size)
            NEPTOOLS_THROW(SourceUnderflow{"Source underflow"} << UsedSource{*this});
        return Pread(offs, buf, len);
    }
    void CheckedPread(FilePosition offs, char* buf, FileMemSize len) const
    { CheckedPread(offs, reinterpret_cast<Byte*>(buf), len); }

    // helper
    uint8_t ReadLittleUint8()
    { return ReadGen<boost::endian::little_uint8_t>(); }
    uint8_t PreadLittleUint8(FilePosition offs) const
    { return PreadGen<boost::endian::little_uint8_t>(offs); }
    uint8_t CheckedReadLittleUint8()
    { return CheckedReadGen<boost::endian::little_uint8_t>(); }
    uint8_t CheckedPreadLittleUint8(FilePosition offs) const
    { return CheckedPreadGen<boost::endian::little_uint8_t>(offs); }

    uint16_t ReadLittleUint16()
    { return ReadGen<boost::endian::little_uint16_t>(); }
    uint16_t PreadLittleUint16(FilePosition offs) const
    { return PreadGen<boost::endian::little_uint16_t>(offs); }
    uint16_t CheckedReadLittleUint16()
    { return CheckedReadGen<boost::endian::little_uint16_t>(); }
    uint16_t CheckedPreadLittleUint16(FilePosition offs) const
    { return CheckedPreadGen<boost::endian::little_uint16_t>(offs); }

    uint32_t ReadLittleUint32()
    { return ReadGen<boost::endian::little_uint32_t>(); }
    uint32_t PreadLittleUint32(FilePosition offs) const
    { return PreadGen<boost::endian::little_uint32_t>(offs); }
    uint32_t CheckedReadLittleUint32()
    { return CheckedReadGen<boost::endian::little_uint32_t>(); }
    uint32_t CheckedPreadLittleUint32(FilePosition offs) const
    { return CheckedPreadGen<boost::endian::little_uint32_t>(offs); }

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


    struct Provider
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
    Source(std::shared_ptr<Provider> p, FilePosition size)
        : size{size}, p{std::move(p)} {}

protected:
    // offset: in original file!
    BufEntry GetTemporaryEntry(FilePosition offs) const;

private:
    static Source FromFile_(boost::filesystem::path fname);

    FilePosition offset = 0, size, get = 0;

    std::shared_ptr<Provider> p;
};

class DumpableSource : public Dumpable, public Source
{
public:
    DumpableSource(const Source& s) : Source{s} {}
    DumpableSource(Source&& s) : Source{std::move(s)} {}
    using Source::Source;

    void Fixup() override {}

    FilePosition GetSize() const override { return Source::GetSize(); }
private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

#define ADD_SOURCE(src)                                 \
    [&](auto& e)                                        \
    {                                                   \
        if (!::boost::get_error_info<UsedSource>(e))    \
            e << UsedSource{src};                       \
    }

}
#endif
