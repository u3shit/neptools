#include "header.hpp"
#include "../context.hpp"
#include "collection_link.hpp"
#include "exports.hpp"
#include <stdexcept>
#include <iostream>

namespace Stcm
{

bool Header::IsValid(size_t file_size) const noexcept
{
    return memcmp(msg.data(), "STCM2L", 6) == 0 &&
        msg.is_valid() &&
        export_offset < file_size - 0x28*export_count &&
        field_28 == 1 &&
        collection_link_offset < file_size;
}

HeaderItem::HeaderItem(Key k, Context* ctx, const Header& hdr)
    : Item{k, ctx}
{
    if (!hdr.IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid STCM header");

    msg = hdr.msg;
    export_sec = ctx->CreateLabelFallback("exports", hdr.export_offset);
    collection_link = ctx->CreateLabelFallback(
        "collection_link_hdr", hdr.collection_link_offset);;
}

HeaderItem* HeaderItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<Header>(ptr);

    auto ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.t);
    CollectionLinkHeaderItem::CreateAndInsert(ret->collection_link->second);
    ExportsItem::CreateAndInsert(ret->export_sec->second, x.t.export_count);
    return ret;
}

void HeaderItem::Dump(std::ostream& os) const
{
    Header hdr;
    hdr.msg = msg;
    hdr.export_offset = ToFilePos(export_sec->second);
    hdr.export_count = export_sec->second.As0<ExportsItem>().entries.size();
    hdr.field_28 = 1;
    hdr.collection_link_offset = ToFilePos(collection_link->second);

    os.write(reinterpret_cast<char*>(&hdr), sizeof(Header));
}

void HeaderItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "Msg: " << msg << "\nVars: @" << export_sec->first << ", @"
       << collection_link->first;
}

}
