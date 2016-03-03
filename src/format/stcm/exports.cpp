#include "exports.hpp"
#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include <iostream>

namespace Stcm
{

void ExportEntry::Validate(FilePosition file_size) const
{
#define VALIDATE(x) VALIDATE_FIELD("Stcm::ExportEntry", x)
    VALIDATE(field_0 == 0);
    VALIDATE(name.is_valid());
    VALIDATE(offset < file_size);
#undef VALIDATE
}

ExportsItem::ExportsItem(Key k, Context* ctx, Source src, uint32_t export_count)
    : Item{k, ctx}
{
    AddInfo(&ExportsItem::Parse_, ADD_SOURCE(src), this, src, export_count);
}

void ExportsItem::Parse_(Source& src, uint32_t export_count)
{
    entries.reserve(export_count);
    auto size = GetContext()->GetSize();
    for (uint32_t i = 0; i < export_count; ++i)
    {
        auto e = src.Read<ExportEntry>();
        e.Validate(size);
        entries.emplace_back(
            e.name,
            GetContext()->CreateLabelFallback(e.name.c_str(), e.offset));
    }
}

ExportsItem* ExportsItem::CreateAndInsert(ItemPointer ptr, uint32_t export_count)
{
    auto x = RawItem::GetSource(ptr, export_count*sizeof(ExportEntry));

    auto ret = x.ritem.SplitCreate<ExportsItem>(
        ptr.offset, x.src, export_count);

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
