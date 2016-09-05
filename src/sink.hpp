#ifndef UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#define UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#pragma once

#include "check.hpp"
#include "nonowning_string.hpp"
#include "shared_ptr.hpp"
#include "utils.hpp"
#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>
#include <cstring>

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(SinkOverflow, std::logic_error);

class Sink : public RefCounted
{
public:
    static NotNull<RefCountedPtr<Sink>> ToFile(
        boost::filesystem::path fname, FilePosition size, bool try_mmap = true);
    static NotNull<RefCountedPtr<Sink>> ToStdOut();

    FilePosition Tell() const noexcept { return offset + buf_put; }

    template <typename Checker = Check::Assert, typename T>
    void WriteGen(const T& x)
    { Write<Checker>({reinterpret_cast<const char*>(&x), EmptySizeof<T>}); }

    template <typename Checker = Check::Assert>
    void Write(StringView data)
    {
        NEPTOOLS_CHECK(SinkOverflow, offset+buf_put+data.length() <= size,
                       "Sink overflow during write");
        auto cp = std::min(data.length(), size_t(buf_size - buf_put));
        memcpy(buf+buf_put, data.data(), cp);
        data.remove_prefix(cp);
        buf_put += cp;

        if (!data.empty()) Write_(data);
    }

    template <typename Checker = Check::Assert>
    void Pad(FileMemSize len)
    {
        NEPTOOLS_CHECK(SinkOverflow, offset+buf_put+len <= size,
                       "Sink overflow during pad");
        auto cp = std::min(len, buf_size - buf_put);
        memset(buf+buf_put, 0, cp);
        buf_put += cp;
        len -= cp;

        if (len) Pad_(len);
    }

    virtual void Flush() {}

#define NEPTOOLS_GEN(bits)                                                  \
    template <typename Checker = Check::Assert>                             \
    void WriteLittleUint##bits (boost::endian::little_uint##bits##_t  i)    \
    { WriteGen<Checker>(i); }
    NEPTOOLS_GEN(8) NEPTOOLS_GEN(16) NEPTOOLS_GEN(32) NEPTOOLS_GEN(64)
#undef NEPTOOLS_GEN

    template <typename Checker = Check::Assert>
    void WriteCString(NonowningString str)
    { Write<Checker>({str.c_str(), str.size()+1}); }

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
