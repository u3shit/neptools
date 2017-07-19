#ifndef UUID_F87DF453_742A_4C38_8660_ABC81ACB04B8
#define UUID_F87DF453_742A_4C38_8660_ABC81ACB04B8
#pragma once

#include "../../source.hpp"
#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stsc
{

class HeaderItem final : public Item
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    struct Header
    {
        char magic[4];
        boost::endian::little_uint32_t entry_point;
        boost::endian::little_uint32_t flags;

        void Validate(FilePosition size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Header) == 12);

    struct ExtraHeader2
    {
        boost::endian::little_uint16_t field_0;
        boost::endian::little_uint16_t field_2;
        boost::endian::little_uint16_t field_4;
        boost::endian::little_uint16_t field_6;
        boost::endian::little_uint16_t field_8;
        boost::endian::little_uint16_t field_a;
        boost::endian::little_uint16_t field_c;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(ExtraHeader2) == 14);


    HeaderItem(Key k, Context& ctx, Source src);
    static HeaderItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override;

    NotNull<LabelPtr> entry_point;
    uint32_t flags = 0;

    std::array<uint8_t, 32> extra_headers_1;

    uint16_t extra_headers_2_0;
    uint16_t extra_headers_2_2;
    uint16_t extra_headers_2_4;
    uint16_t extra_headers_2_6;
    uint16_t extra_headers_2_8;
    uint16_t extra_headers_2_a;
    uint16_t extra_headers_2_c;

    uint16_t extra_headers_4;

private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
};

}
}

#endif
