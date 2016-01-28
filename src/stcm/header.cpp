#include "../context.hpp"
#include "header.hpp"
#include <stdexcept>
#include <iostream>

namespace Stcm
{

bool Header::IsValid(size_t file_size)
{
    return memcmp(msg, "STCM2L", 6) == 0 &&
        parts.rest[sizeof(parts.rest)-1] == 0 &&
        export_offset < file_size - 0x28*export_count &&
        field_28 == 1 &&
        collection_link_offset < file_size;
}

HeaderItem::HeaderItem(Key k, Context* ctx, const Byte* data, size_t len)
    : Item{k, ctx}
{
    if (len < sizeof(Header))
        throw std::out_of_range("STCM header too short");
    memcpy(&raw, data, sizeof(Header));
    if (!raw.IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid STCM header");
}

void HeaderItem::Dump(std::ostream& os) const
{
    os << "Msg: " << raw.msg << "\nVars: " << raw.export_offset << ", "
       << raw.export_count << ", " << raw.field_28 << ", "
       << raw.collection_link_offset;
}

}
