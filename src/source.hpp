#ifndef UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#define UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#pragma once

#include <array>
#include <boost/endian/arithmetic.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/get_error_info.hpp>
#include "dumpable.hpp"
#include "fs.hpp"
#include "low_io.hpp"

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

    static Source FromFile(fs::path fname);

    void Slice(FilePosition offset, FilePosition size) noexcept
    {
        BOOST_ASSERT(offset <= this->size &&
                     offset + size <= this->size);
        this->offset += offset;
        this->get -= offset;
        this->size = size;
    }

    FilePosition GetOffset() const noexcept { return offset; }
    FilePosition GetOrigSize() const noexcept { return p->size; }
    const fs::path& GetFileName() const noexcept { return p->file_name; }

    FilePosition GetSize() const noexcept { return size; }

    void Seek(FilePosition pos) noexcept
    {
        BOOST_ASSERT(pos <= size);
        get = pos;
    }
    FilePosition Tell() const noexcept { return get; }
    bool Eof() const noexcept { return get == size; }

    template <typename T>
    void Read(T& x) { Read(reinterpret_cast<Byte*>(&x), sizeof(T)); }
    template <typename T>
    T Read() { T ret; Read(ret); return ret; }

    template <typename T>
    void Pread(FilePosition offs, T& x) const
    { Pread(offs, reinterpret_cast<Byte*>(&x), sizeof(T)); }
    template <typename T>
    T Pread(FilePosition offs) const
    { T ret; Pread(offs, ret); return ret; }

    void Read(Byte* buf, FileMemSize len) { Pread(get, buf, len); get += len; }
    void Read(char* buf, FileMemSize len) { Pread(get, buf, len); get += len; }
    void Pread(FilePosition offs, Byte* buf, FileMemSize len) const;
    void Pread(FilePosition offs, char* buf, FileMemSize len) const
    { Pread(offs, reinterpret_cast<Byte*>(buf), len); }

    Byte Get() { return Read<Byte>(); }
    Byte Get(uint64_t offs) const { return Pread<Byte>(offs); }

    // helper
    uint8_t GetLittleUint8() { return Get(); }
    uint8_t GetLittleUint8(FilePosition offs) const { return Get(offs); }

    uint16_t GetLittleUint16()
    { return Read<boost::endian::little_uint16_t>(); }
    uint16_t GetLittleUint16(FilePosition offs) const
    { return Pread<boost::endian::little_uint16_t>(offs); }

    uint32_t GetLittleUint32()
    { return Read<boost::endian::little_uint32_t>(); }
    uint32_t GetLittleUint32(FilePosition offs) const
    { return Pread<boost::endian::little_uint32_t>(offs); }

    std::string GetCstring()
    {
        std::string str;
        while (char c = Get())
            str += c;
        return str;
    }

    std::string GetCstring(FilePosition offs) const
    {
        std::string str;
        while (char c = Get(offs++))
            str += c;
        return str;
    }


    struct Provider
    {
        Provider(fs::path file_name, FilePosition size)
            : file_name{std::move(file_name)}, size{size} {}
        Provider(const Provider&) = delete;
        void operator=(const Provider&) = delete;
        virtual ~Provider() = default;

        virtual void Pread(FilePosition offs, Byte* buf, FileMemSize len) = 0;

        void LruPush(Byte* ptr, FilePosition offset, FileMemSize size);
        bool LruGet(FilePosition offs);

        std::array<BufEntry, 4> lru;
        fs::path file_name;
        FilePosition size;
    };
    Source(std::shared_ptr<Provider> p, FilePosition size)
        : size{size}, p{std::move(p)} {}

protected:
    // offset: in original file!
    BufEntry GetTemporaryEntry(FilePosition offs) const;

private:
    static Source FromFile_(fs::path fname);

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

using UsedSource = boost::error_info<struct UsedSourceTag, Source>;
using ReadOffset = boost::error_info<struct ReadOffsetTag, FilePosition>;
using ReadSize = boost::error_info<struct ReadOffsetTag, FileMemSize>;
std::string to_string(const UsedSource& src);

#endif
