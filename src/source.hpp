#ifndef UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#define UUID_11A3E8B0_C5C5_4C4E_A22E_56F6E5346CEC
#pragma once

#include <array>
#include <boost/endian/arithmetic.hpp>
#include "dumpable.hpp"

/// A fixed size, read-only, seekable data source (or something that emulates it)
class Source
{
public:
    struct BufEntry
    {
        Byte* ptr = nullptr;
        uint64_t offset = -1;
        size_t size = 0;
    };

    Source(const Source& s, uint64_t offset, uint64_t size)
        : Source{s} { Slice(offset, size); get = 0; }

    static Source FromFile(fs::path fname);

    void Slice(uint64_t offset, uint64_t size)
    {
        BOOST_ASSERT(offset <= this->size &&
                     offset + size <= this->size);
        this->offset += offset;
        this->get -= offset;
        this->size = size;
    }

    uint64_t GetOffset() const noexcept { return offset; }
    uint64_t GetOrigSize() const noexcept { return p->size; }
    const fs::path& GetFileName() const noexcept { return p->file_name; }

    uint64_t GetSize() const noexcept { return size; }

    void Seek(uint64_t pos) noexcept
    {
        BOOST_ASSERT(pos <= size);
        get = pos;
    }
    uint64_t Tell() const noexcept { return get; }
    bool Eof() const noexcept { return get == size; }

    // offset: in original file!
    BufEntry GetTemporaryEntry(uint64_t offs) const
    {
        if (GetEntry(offs)) return p->lru[0];
        p->Pread(offs, nullptr, 0);
        BOOST_ASSERT(p->lru[0].offset <= offs &&
                     p->lru[0].offset + p->lru[0].size > offs);
        return p->lru[0];
    }

    template <typename T>
    void Read(T& x) { Read(reinterpret_cast<Byte*>(&x), sizeof(T)); }
    template <typename T>
    T Read() { T ret; Read(ret); return ret; }

    template <typename T>
    void Pread(uint64_t offs, T& x) const
    { Pread(offs, reinterpret_cast<Byte*>(&x), sizeof(T)); }
    template <typename T>
    T Pread(uint64_t offs) const
    { T ret; Pread(offs, ret); return ret; }

    void Read(Byte* buf, size_t len) { Pread(get, buf, len); get += len; }
    void Read(char* buf, size_t len) { Pread(get, buf, len); get += len; }
    void Pread(uint64_t offs, Byte* buf, size_t len) const
    {
        BOOST_ASSERT(offs <= size && offs+len <= size);
        offs += offset;
        while (len)
        {
            if (GetEntry(offs))
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
    void Pread(uint64_t offs, char* buf, size_t len) const
    { Pread(offs, reinterpret_cast<Byte*>(buf), len); }

    Byte Get() { return Read<Byte>(); }
    Byte Get(uint64_t offs) const { return Pread<Byte>(offs); }

    // helper
    uint8_t GetLittleUint8() { return Get(); }
    uint8_t GetLittleUint8(uint64_t offs) const { return Get(offs); }

    uint16_t GetLittleUint16()
    { return Read<boost::endian::little_uint16_t>(); }
    uint16_t GetLittleUint16(uint64_t offs) const
    { return Pread<boost::endian::little_uint16_t>(offs); }

    uint32_t GetLittleUint32()
    { return Read<boost::endian::little_uint32_t>(); }
    uint32_t GetLittleUint32(uint64_t offs) const
    { return Pread<boost::endian::little_uint32_t>(offs); }

    std::string GetCstring()
    {
        std::string str;
        while (char c = Get())
            str += c;
        return str;
    }

    std::string GetCstring(uint64_t offs) const
    {
        std::string str;
        while (char c = Get(offs++))
            str += c;
        return str;
    }

private:
    struct SourceProvider;
    Source(std::shared_ptr<SourceProvider> p, uint64_t size)
        : size{size}, p{std::move(p)} {}

    uint64_t offset = 0, size, get = 0;
    bool GetEntry(uint64_t offs) const
    {
        for (size_t i = 0; i < p->lru.size(); ++i)
        {
            auto x = p->lru[i];
            if (x.offset <= offs && x.offset + x.size > offs)
            {
                memmove(&p->lru[1], &p->lru[0], sizeof(BufEntry)*i);
                p->lru[0] = x;
                return true;
            }
        }
        return false;
    }

    struct SourceProvider
    {
        SourceProvider(fs::path file_name, uint64_t size)
            : file_name{std::move(file_name)}, size{size} {}
        SourceProvider(const SourceProvider&) = delete;
        void operator=(const SourceProvider&) = delete;
        virtual ~SourceProvider() = default;

        virtual void Pread(uint64_t offs, Byte* buf, size_t len) = 0;

        std::array<BufEntry, 4> lru;
        fs::path file_name;
        uint64_t size;
    };
    std::shared_ptr<SourceProvider> p;

    template <typename T>
    struct UnixLike : public SourceProvider
    {
        UnixLike(int fd, fs::path file_name, uint64_t size)
            : SourceProvider{std::move(file_name), size}, fd{fd} {}
        ~UnixLike();

        void Pread(uint64_t offs, Byte* buf, size_t len) override;
        void EnsureChunk(uint64_t i);

        int fd;
    };

    struct MmapProvider final : public UnixLike<MmapProvider>
    {
        MmapProvider(int fd, fs::path file_name, uint64_t size);

        static size_t CHUNK_SIZE;
        void* ReadChunk(uint64_t offs, size_t size);
        void DeleteChunk(size_t i);
    };

    struct UnixProvider final : public UnixLike<MmapProvider>
    {
        using UnixLike::UnixLike;

        static size_t CHUNK_SIZE;
        void* ReadChunk(uint64_t offs, size_t size);
        void DeleteChunk(size_t i);
    };
};

class DumpableSource : public Dumpable, public Source
{
public:
    DumpableSource(const Source& s) : Source{s} {}
    DumpableSource(Source&& s) : Source{std::move(s)} {}
    using Source::Source;

    void Fixup() override {}

    uint64_t GetSize() const override { return Source::GetSize(); }
private:
    void Dump_(std::ostream& os) const override;
    void Inspect_(std::ostream& os) const override;
};

#endif
