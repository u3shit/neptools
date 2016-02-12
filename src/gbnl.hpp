#ifndef GBNL_HPP
#define GBNL_HPP
#pragma once

#include "dumpable.hpp"
#include "buffer.hpp"
#include "dynamic_struct.hpp"
#include <boost/endian/arithmetic.hpp>
#include <vector>

struct GbnlTypeDescriptor
{
    enum Type
    {
        UINT32 = 0,
        UINT8  = 1,
        UINT16 = 2,
        FLOAT  = 3,
        STRING = 5,
    };
    boost::endian::little_uint16_t type;
    boost::endian::little_uint16_t offset;
};
static_assert(sizeof(GbnlTypeDescriptor) == 0x04, "");

struct GbnlFooter
{
    char magic[3];
    char version;

    boost::endian::little_uint32_t field_04;
    boost::endian::little_uint32_t field_08;
    boost::endian::little_uint32_t field_0c;
    boost::endian::little_uint32_t flags; // 1 if there's a string, 0 otherwise?
    boost::endian::little_uint32_t descr_offset;
    boost::endian::little_uint32_t count_msgs;
    boost::endian::little_uint32_t msg_descr_size;
    boost::endian::little_uint16_t count_types;
    boost::endian::little_uint16_t field_22;
    boost::endian::little_uint32_t offset_types;
    boost::endian::little_uint32_t field_28;
    boost::endian::little_uint32_t offset_msgs;
    boost::endian::little_uint32_t field_30;
    boost::endian::little_uint32_t field_34;
    boost::endian::little_uint32_t field_38;
    boost::endian::little_uint32_t field_3c;

    bool IsValid(size_t chunk_size) const noexcept;
};
static_assert(sizeof(GbnlFooter) == 0x40, "");

class Gbnl : public Dumpable
{
public:
    Gbnl(const Byte* data, size_t len);
    Gbnl(const Buffer* buf) : Gbnl{buf->GetPtr(), buf->GetSize()} {}

    void Fixup() override { RecalcSize(); }

    void WriteTxt(std::ostream& os) const;
    void WriteTxt(std::ostream&& os) const { WriteTxt(os); }
    void ReadTxt(std::istream& is);
    void ReadTxt(std::istream&& is) { ReadTxt(is); }

    struct OffsetString
    {
        std::string str;
        size_t offset;
    };

    struct FixStringTag { char str[1]; };
    using Struct = DynamicStruct<
        uint8_t, uint16_t, uint32_t, float, OffsetString, FixStringTag>;

    uint32_t flags, field_28, field_30;
    std::vector<Struct> messages;
    Struct::TypePtr type;

    void RecalcSize();
    size_t GetSize() const noexcept;

private:
    void Dump_(std::ostream& os) const override;
    void Inspect_(std::ostream& os) const override;

    size_t msg_descr_size, msgs_size;
};


#endif
