#include "exports.hpp"
#include "header.hpp"
#include "../context.hpp"
#include <iostream>

namespace Stcm
{

bool ExportEntry::IsValid(size_t file_size) const noexcept
{
    return field_0 == 0 && name.is_valid() && offset < file_size;
}

Exports::Exports(Key k, Context* ctx, const HeaderItem* hdr)
    : Item{k, ctx}
{
    auto& ptr = hdr->export_sec->second;
    auto& ritem = dynamic_cast<RawItem&>(*ptr.item);
    auto e = reinterpret_cast<const ExportEntry*>(ritem.GetPtr() + ptr.offset);

    entries.reserve(hdr->export_count);
    auto size = GetContext()->GetSize();
    for (size_t i = 0; i < hdr->export_count; ++i)
    {
        if (!e[i].IsValid(size))
            throw std::runtime_error("Invalid export entry");
        entries.emplace_back(
            e[i].name,
            GetContext()->CreateLabelFallback(e[i].name.c_str(), e[i].offset));
    }
}

void Exports::Dump(std::ostream& os) const
{
    Item::Dump(os);

    for (auto& e : entries)
        os << e.first << " -> @" << e.second->first << '\n';
}

}
