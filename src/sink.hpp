#ifndef UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#define UUID_8EC8FF70_7F93_4281_9370_FF756B846775
#pragma once

#include "check.hpp"
#include "nonowning_string.hpp"
#include "shared_ptr.hpp"
#include "utils.hpp"
#include "lua/dynamic_object.hpp"

#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>
#include <cstring>

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(SinkOverflow, std::logic_error);

class Sink : public RefCounted, public Lua::DynamicObject
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    static NotNull<RefCountedPtr<Sink>> ToFile(
        boost::filesystem::path fname, FilePosition size, bool try_mmap = true);
    static NotNull<RefCountedPtr<Sink>> ToStdOut();

    FilePosition Tell() const noexcept { return offset + buf_put; }

    template <typename Checker = Check::Assert, typename T>
    NEPTOOLS_NOLUA void WriteGen(const T& x)
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
} NEPTOOLS_LUAGEN(post_register=[[
    // hack to get close call __gc
    lua_getfield(vm, -2, "__gc");
    bld.SetField("close");
]]);

class MemorySink final : public Sink
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:

    NEPTOOLS_NOLUA
    MemorySink(Byte* buffer, FileMemSize size) : Sink{size}
    { this->buf = buffer; this->buf_size = size; }

    NEPTOOLS_NOLUA
    MemorySink(std::unique_ptr<Byte[]> buffer, FileMemSize size)
        : Sink{size}, uniq_buf{std::move(buffer)}
    { this->buf = uniq_buf.get(); this->buf_size = size; }

    MemorySink(FileMemSize size) : Sink{size}, uniq_buf{new Byte[size]}
    { buf = uniq_buf.get(); buf_size = size; }

    NEPTOOLS_LUAGEN(name="to_string")
    StringView GetStringView() const noexcept { return {buf, buf_size}; }

    NEPTOOLS_NOLUA
    std::unique_ptr<Byte[]> Release() noexcept { return std::move(uniq_buf); }

private:
    std::unique_ptr<Byte[]> uniq_buf;

    void Write_(StringView) override;
    void Pad_(FileMemSize) override;
};

}
#endif
