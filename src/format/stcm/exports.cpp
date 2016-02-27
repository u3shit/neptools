#include "exports.hpp"
#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include <iostream>

namespace Stcm
{

bool ExportEntry::IsValid(size_t file_size) const noexcept
{
    return field_0 == 0 && name.is_valid() && offset < file_size;
}

ExportsItem::ExportsItem(Key k, Context* ctx, Source src, size_t export_count)
    : Item{k, ctx}
{
    entries.reserve(export_count);
    auto size = GetContext()->GetSize();
    for (size_t i = 0; i < export_count; ++i)
    {
        auto e = src.Read<ExportEntry>();
        if (!e.IsValid(size))
            throw std::runtime_error("Invalid export entry");
        entries.emplace_back(
            e.name,
            GetContext()->CreateLabelFallback(e.name.c_str(), e.offset));
    }
}

ExportsItem* ExportsItem::CreateAndInsert(ItemPointer ptr, size_t export_count)
{
    auto x = RawItem::GetSource(ptr, export_count*sizeof(ExportEntry));

    auto ret = x.ritem.SplitCreate<ExportsItem>(ptr.offset, x.src, export_count);

    for (const auto& e : ret->entries)
        MaybeCreate<InstructionItem>(e.second->second);
    return ret;
}

void ExportsItem::Dump(std::ostream& os) const
{
    ExportEntry ee;
    ee.field_0 = 0;

    for (auto& e : entries)
    {
        ee.name = e.first;
        ee.offset = ToFilePos(e.second->second);
        os.write(reinterpret_cast<char*>(&ee), sizeof(ExportEntry));
    }
}

void ExportsItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    for (auto& e : entries)
        os << e.first << " -> @" << e.second->first << '\n';
}

}
