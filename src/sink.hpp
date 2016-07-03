#ifndef UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#define UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#pragma once

#include "assert.hpp"
#include "nonowning_string.hpp"
#include "utils.hpp"
#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>
#include <cstring>

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(SinkOverflow, std::logic_error);

class Sink
{
public:
    virtual ~Sink() = default;
    static std::unique_ptr<Sink> ToFile(
        boost::filesystem::path fname, FilePosition size, bool try_mmap = true);
    static std::unique_ptr<Sink> ToStdOut();

    FilePosition Tell() const noexcept { return offset + buf_put; }

    template <typename T>
    void WriteGen(const T& x)
    { Write({reinterpret_cast<const char*>(&x), EmptySizeof<T>}); }

    template <typename T>
    void CheckedWriteGen(const T& x)
    { CheckedWrite({reinterpret_cast<const char*>(&x), EmptySizeof<T>}); }

    void Write(StringView data)
    {
        NEPTOOLS_ASSERT_MSG(offset+buf_put+data.length() <= size, "sink overflow");
        auto cp = std::min(data.length(), size_t(buf_size - buf_put));
        memcpy(buf+buf_put, data.data(), cp);
        data.remove_prefix(cp);
        buf_put += cp;

        if (!data.empty()) Write_(data);
    }

    void CheckedWrite(StringView data)
    {
        if (offset+buf_put+data.length() > size)
            NEPTOOLS_THROW(SinkOverflow{"Sink overflow"});
        Write(data);
    }

    void Pad(FileMemSize len)
    {
        NEPTOOLS_ASSERT_MSG(offset+buf_put+len <= size, "sink overflow");
        auto cp = std::min(len, buf_size - buf_put);
        memset(buf+buf_put, 0, cp);
        buf_put += cp;
        len -= cp;

        if (len) Pad_(len);
    }

    void CheckedPad(FileMemSize len)
    {
        if (offset+buf_put+len > size)
            NEPTOOLS_THROW(SinkOverflow{"Sink overflow"});
        Pad(len);
    }

    virtual void Flush() {}

    void WriteLittleUint8 (boost::endian::little_uint8_t  i) { WriteGen(i); }
    void WriteLittleUint16(boost::endian::little_uint16_t i) { WriteGen(i); }
    void WriteLittleUint32(boost::endian::little_uint32_t i) { WriteGen(i); }
    void WriteLittleUint64(boost::endian::little_uint64_t i) { WriteGen(i); }
    void WriteCString(NonowningString str)
    { Write({str.c_str(), str.size()+1}); }

    void CheckedWriteLittleUint8 (boost::endian::little_uint8_t  i)
    { CheckedWriteGen(i); }
    void CheckedWriteLittleUint16(boost::endian::little_uint16_t i)
    { CheckedWriteGen(i); }
    void CheckedWriteLittleUint32(boost::endian::little_uint32_t i)
    { CheckedWriteGen(i); }
    void CheckedWriteLittleUint64(boost::endian::little_uint64_t i)
    { CheckedWriteGen(i); }
    void CheckedWriteCString(NonowningString str)
    { CheckedWrite({str.c_str(), str.size()+1}); }

protected:
    Sink(FileMemSize size) : size{size} {}

    Byte* buf = nullptr;
    FilePosition offset = 0, size;
    FileMemSize buf_put = 0, buf_size;

private:
    virtual void Write_(StringView data) = 0;
    virtual void Pad_(FileMemSize len) = 0;
};

class MemorySink : public Sink
{
public:
    MemorySink(Byte* buffer, FileMemSize size) : Sink{size}
    { this->buf = buffer; this->buf_size = size; }

private:
    void Write_(StringView) override;
    void Pad_(FileMemSize) override;
};

}
#endif
