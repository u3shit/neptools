#ifndef STCM_HEADER_HPP
#define STCM_HEADER_HPP
#pragma once

#include "../fixed_string.hpp"
#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

struct Header
{
    struct MsgParts
    {
        char magic[5];
        char version;
        char rest[0x20-5-1];
    };
    union
    {
        FixedString<0x20> msg;
        MsgParts parts;
    };
    boost::endian::little_uint32_t export_offset;
    boost::endian::little_uint32_t export_count;
    boost::endian::little_uint32_t field_28;
    boost::endian::little_uint32_t collection_link_offset;

    bool IsValid(size_t file_size) const noexcept;
};
static_assert(sizeof(Header) == 0x30, "");

class HeaderItem final : public Item
{
public:
    HeaderItem(Key k, Context* ctx, const Header* hdr);
    static HeaderItem* CreateAndInsert(RawItem* ritem);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override { return sizeof(Header); }

    FixedString<0x20> msg;
    const Label* export_sec;
    const Label* collection_link;
};

}

#endif
