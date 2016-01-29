#include "../context.hpp"
#include "header.hpp"
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
    collection_link = ctx->CreateLabelFallback("collection_link", raw->collection_link_offset);;
}

HeaderItem* HeaderItem::CreateAndInsert(Context* ctx, RawItem* ritem)
{
    if (ritem->GetSize() < sizeof(Header))
        throw std::out_of_range("STCM header too short");
    auto raw = reinterpret_cast<const Header*>(ritem->GetPtr());

    return ritem->Split(0, ctx->Create<HeaderItem>(raw));
}

void HeaderItem::Dump(std::ostream& os) const
{
    Item::Dump(os);

    os << "Msg: " << msg << "\nVars: @" << export_sec->first << ", "
       << export_count << ", @"
       << collection_link->first;
}

}
