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

ExportsItem::ExportsItem(Key k, Context* ctx, const ExportEntry* e, size_t export_count)
    : Item{k, ctx}
{
    entries.reserve(export_count);
    auto size = GetContext()->GetSize();
    for (size_t i = 0; i < export_count; ++i)
    {
        if (!e[i].IsValid(size))
            throw std::runtime_error("Invalid export entry");
        entries.emplace_back(
            e[i].name,
            GetContext()->CreateLabelFallback(e[i].name.c_str(), e[i].offset));
    }
}

ExportsItem* ExportsItem::CreateAndInsert(Context* ctx, const HeaderItem* hdr)
{
    auto& ptr = hdr->export_sec->second;
    auto& ritem = dynamic_cast<RawItem&>(*ptr.item);
    auto e = reinterpret_cast<const ExportEntry*>(ritem.GetPtr() + ptr.offset);

    if (ritem.GetSize() - ptr.offset < hdr->export_count*sizeof(ExportEntry))
        throw std::runtime_error("Invalid export entry: premature end of data");
    return ritem.Split(ptr.offset, ctx->Create<ExportsItem>(e, hdr->export_count));
}

void ExportsItem::Dump(std::ostream& os) const
{
    Item::Dump(os);

    for (auto& e : entries)
        os << e.first << " -> @" << e.second->first << '\n';
}

}
