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

HeaderItem::HeaderItem(Key k, Context* ctx, const Header* raw)
    : Item{k, ctx}
{
    if (!raw->IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid STCM header");

    msg = raw->msg;
    export_sec = ctx->CreateLabelFallback("exports", raw->export_offset);
    export_count = raw->export_count;
    collection_link = ctx->CreateLabelFallback(
        "collection_link_hdr", raw->collection_link_offset);;
}

HeaderItem* HeaderItem::CreateAndInsert(RawItem* ritem)
{
    if (ritem->GetSize() < sizeof(Header))
        throw std::out_of_range("STCM header too short");
    auto raw = reinterpret_cast<const Header*>(ritem->GetPtr());

    auto ret = ritem->Split(0, ritem->GetContext()->Create<HeaderItem>(raw));
    CollectionLinkHeaderItem::CreateAndInsert(ret->collection_link->second);
    ExportsItem::CreateAndInsert(ret);
    return ret;
}

void HeaderItem::Dump(std::ostream& os) const
{
    Header hdr;
    hdr.msg = msg;
    hdr.export_offset = ToFilePos(export_sec->second);
    hdr.export_count = export_count;
    hdr.field_28 = 1;
    hdr.collection_link_offset = ToFilePos(collection_link->second);

    os.write(reinterpret_cast<char*>(&hdr), sizeof(Header));
}

void HeaderItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "Msg: " << msg << "\nVars: @" << export_sec->first << ", "
       << export_count << ", @"
       << collection_link->first;
}

}
