#include "header.hpp"
#include "../context.hpp"
#include "collection_link.hpp"
#include "exports.hpp"
#include <stdexcept>
#include <iostream>

namespace Neptools
{
namespace Stcm
{

void HeaderItem::Header::Validate(FilePosition file_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm::HeaderItem::Header", x)
    VALIDATE(memcmp(msg.data(), "STCM2L", 6) == 0);
    VALIDATE(msg.is_valid());
    VALIDATE(export_offset < file_size - 0x28*export_count);
    VALIDATE(collection_link_offset < file_size);
#undef VALIDATE
}

HeaderItem::HeaderItem(Key k, Context* ctx, const Header& hdr)
    : Item{k, ctx}
{
    hdr.Validate(GetContext()->GetSize());

    msg = hdr.msg;
    export_sec = ctx->CreateLabelFallback("exports", hdr.export_offset);
    collection_link = ctx->CreateLabelFallback(
        "collection_link_hdr", hdr.collection_link_offset);;
    field_28 = hdr.field_28;
}

HeaderItem* HeaderItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<Header>(ptr);

    auto ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.t);
    CollectionLinkHeaderItem::CreateAndInsert(ret->collection_link->second);
    ExportsItem::CreateAndInsert(ret->export_sec->second, x.t.export_count);
    return ret;
}

void HeaderItem::Dump_(Sink& sink) const
{
    Header hdr;
    hdr.msg = msg;
    hdr.export_offset = ToFilePos(export_sec->second);
    hdr.export_count = export_sec->second.As0<ExportsItem>().entries.size();
    hdr.field_28 = field_28;
    hdr.collection_link_offset = ToFilePos(collection_link->second);

    sink.WriteGen(hdr);
}

void HeaderItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);

    os << "stcm_header(";
    DumpBytes(os, msg.c_str());
    os << ", @" << export_sec->first << ", @" << collection_link->first
       << ", " << field_28 << ")";
}

}
}
