#ifndef UUID_9037C300_D4EF_473C_8387_A1A9797069A7
#define UUID_9037C300_D4EF_473C_8387_A1A9797069A7
#pragma once

#include "../dumpable.hpp"
#include "../source.hpp"
#include "../dynamic_struct.hpp"
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
STATIC_ASSERT(sizeof(GbnlTypeDescriptor) == 0x04);

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
STATIC_ASSERT(sizeof(GbnlFooter) == 0x40);

class Gbnl : public Dumpable
{
public:
    Gbnl(Source src);
    //Gbnl(const Byte* data, size_t len);

    void Fixup() override { RecalcSize(); }

    void WriteTxt(std::ostream& os) const;
    void WriteTxt(std::ostream&& os) const { WriteTxt(os); }
    void ReadTxt(std::istream& is);
    void ReadTxt(std::istream&& is) { ReadTxt(is); }

    struct OffsetString
    {
        std::string str;
        uint32_t offset;
    };

    struct FixStringTag { char str[1]; };
    using Struct = DynamicStruct<
        uint8_t, uint16_t, uint32_t, float, OffsetString, FixStringTag>;

    bool is_gstl;
    uint32_t flags, field_28, field_30;
    std::vector<Struct> messages;
    Struct::TypePtr type;

    void RecalcSize();
    FilePosition GetSize() const noexcept override;

private:
    void Dump_(std::ostream& os) const override;
    void DumpHeader(std::ostream& os) const;
    void Inspect_(std::ostream& os) const override;
    FilePosition Align(FilePosition x) const noexcept;

    uint32_t GetId(const Gbnl::Struct& m, size_t i, size_t j, size_t& k) const;
    size_t FindDst(uint32_t id, std::vector<Gbnl::Struct>& messages,
                   size_t& index) const;

    size_t msg_descr_size, msgs_size;
};


#endif
