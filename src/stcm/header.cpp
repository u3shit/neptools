#include "header.hpp"
#include <stdexcept>

namespace Stcm
{

HeaderItem::HeaderItem(const Byte* data, size_t len)
{
    if (len < sizeof(Header))
        throw std::out_of_range("STCM header too short");
    memcpy(&raw, data, sizeof(Header));
}

void HeaderItem::Dump(std::ostream& os) const
{
    os << "Msg: " << raw.msg << "\nVars: " << raw.export_offset << ", "
       << raw.export_count << ", " << raw.field_28 << ", "
       << raw.collection_link_offset;
}

}
