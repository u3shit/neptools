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

HeaderItem::HeaderItem(Key k, Context* ctx, const Byte* data, size_t len)
    : Item{k, ctx}
{
    if (len < sizeof(Header))
        throw std::out_of_range("STCM header too short");
    auto raw = reinterpret_cast<const Header*>(data);
    if (!raw->IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid STCM header");

    msg = raw->msg;
    export_sec = ctx->CreateLabelFallback("exports", raw->export_offset);
    export_count = raw->export_count;
    field_28 = raw->field_28;
    collection_link = ctx->CreateLabelFallback("collection_link", raw->collection_link_offset);;
}

void HeaderItem::Dump(std::ostream& os) const
{
    os << "Msg: " << msg << "\nVars: " << export_sec->first << ", "
       << export_count << ", " << field_28 << ", "
       << collection_link->first;
}

}
