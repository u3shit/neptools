#ifndef UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#define UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#pragma once

#include "utils.hpp"
#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>
#include <cstring>

namespace Neptools
{

class Sink
{
public:
    virtual ~Sink() = default;
    static std::unique_ptr<Sink> ToFile(
        boost::filesystem::path fname, FilePosition size, bool try_mmap = true);
    static std::unique_ptr<Sink> ToStdOut();

    FilePosition Tell() const noexcept { return offset + buf_put; }

    template <typename T>
    void Write(const T& x)
    { Write(reinterpret_cast<const Byte*>(&x), sizeof(T)); }
    void Write(const char* data, FileMemSize len)
    { Write(reinterpret_cast<const Byte*>(data), len); }

    void Write(const Byte* data, FileMemSize len)
    {
        NEPTOOLS_ASSERT_MSG(offset+buf_put+len <= size, "sink overflow");
        auto cp = std::min(len, buf_size - buf_put);
        memcpy(buf+buf_put, data, cp);
        data += cp;
        buf_put += cp;
        len -= cp;

        if (len) Write_(data, len);
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

    virtual void Flush() {}

    void WriteLittleUint8 (boost::endian::little_uint8_t  i) { Write(i); }
    void WriteLittleUint16(boost::endian::little_uint16_t i) { Write(i); }
    void WriteLittleUint32(boost::endian::little_uint32_t i) { Write(i); }
    void WriteCString(const std::string& str) { Write(str.c_str(), str.size()+1); }
    void WriteCString(const char* str) { Write(str, strlen(str)+1); }

protected:
    Sink(FileMemSize size) : size{size} {}

    Byte* buf = nullptr;
    FilePosition offset = 0, size;
    FileMemSize buf_put = 0, buf_size;

private:
    virtual void Write_(const Byte* buf, FileMemSize len) = 0;
    virtual void Pad_(FileMemSize len) = 0;
};

class MemorySink : public Sink
{
public:
    MemorySink(Byte* buffer, FileMemSize size) : Sink{size}
    { this->buf = buffer; this->buf_size = size; }

private:
    void Write_(const Byte*, FileMemSize) override;
    void Pad_(FileMemSize) override;
};

}
#endif
