#ifndef STCM_HEADER_HPP
#define STCM_HEADER_HPP
#pragma once

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
        char msg[0x20];
        MsgParts parts;
    };
    boost::endian::little_uint32_t export_offset;
    boost::endian::little_uint32_t export_count;
    boost::endian::little_uint32_t field_28;
    boost::endian::little_uint32_t collection_link_offset;

    bool IsValid(size_t file_size);
};
static_assert(sizeof(Header) == 0x30, "");

class HeaderItem : public Item
{
public:
    HeaderItem(ContextKey ctx, const RawItem& it)
        : HeaderItem{ctx, it.GetPtr(), it.GetSize()} {}
    HeaderItem(ContextKey ctx, const Byte* data, size_t len);

    void Dump(std::ostream& os) const;
    size_t GetSize() const noexcept { return sizeof(Header); }
private:
    Header raw;
};

}

#endif
