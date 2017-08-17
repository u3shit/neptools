#ifndef UUID_B9D3C4DA_158C_4858_903C_9EBDD92C2CBC
#define UUID_B9D3C4DA_158C_4858_903C_9EBDD92C2CBC
#pragma once

#include "../raw_item.hpp"
#include <libshit/fixed_string.hpp>
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stcm
{

class HeaderItem final : public Item
{
    LIBSHIT_DYNAMIC_OBJECT;
public:
    using MsgType = Libshit::FixedString<0x20-5-1>;
    struct Header
    {
        char magic[5];
        char endian;
        MsgType msg;

        boost::endian::little_uint32_t export_offset;
        boost::endian::little_uint32_t export_count;
        boost::endian::little_uint32_t field_28;
        boost::endian::little_uint32_t collection_link_offset;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Header) == 0x30);

    HeaderItem(
        Key k, Context& ctx, const MsgType& msg,
        Libshit::NotNull<LabelPtr> export_sec,
        Libshit::NotNull<LabelPtr> collection_link, uint32_t field_28)
        : Item{k, ctx}, msg{msg}, export_sec{std::move(export_sec)},
          collection_link{std::move(collection_link)}, field_28{field_28} {}
    LIBSHIT_NOLUA
    HeaderItem(Key k, Context& ctx, const Header& hdr);
    static HeaderItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override { return sizeof(Header); }

    MsgType msg;
    Libshit::NotNull<LabelPtr> export_sec;
    Libshit::NotNull<LabelPtr> collection_link;
    uint32_t field_28;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
};

}
}
#endif
