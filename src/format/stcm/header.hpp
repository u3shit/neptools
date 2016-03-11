#ifndef UUID_B9D3C4DA_158C_4858_903C_9EBDD92C2CBC
#define UUID_B9D3C4DA_158C_4858_903C_9EBDD92C2CBC
#pragma once

#include "../../fixed_string.hpp"
#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

class HeaderItem final : public Item
{
public:
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

        void Validate(FilePosition file_size) const;
    };
    STATIC_ASSERT(sizeof(Header) == 0x30);


    HeaderItem(Key k, Context* ctx, const Header& hdr);
    static HeaderItem* CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override { return sizeof(Header); }

    FixedString<0x20> msg;
    const Label* export_sec;
    const Label* collection_link;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

}

#endif
