#include "exports.hpp"
#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include <iostream>

namespace Neptools
{
namespace Stcm
{

void ExportsItem::Entry::Validate(FilePosition file_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm::ExportsItem::Entry", x)
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
        auto e = src.ReadGen<Entry>();
        e.Validate(size);
        entries.emplace_back(
            e.name,
            GetContext()->CreateLabelFallback(e.name.c_str(), e.offset));
    }
}

ExportsItem* ExportsItem::CreateAndInsert(ItemPointer ptr, uint32_t export_count)
{
    auto x = RawItem::GetSource(ptr, export_count*sizeof(Entry));

    auto ret = x.ritem.SplitCreate<ExportsItem>(
        ptr.offset, x.src, export_count);

    for (const auto& e : ret->entries)
        MaybeCreate<InstructionItem>(e.second->second);
    return ret;
}

void ExportsItem::Dump_(Sink& sink) const
{
    Entry ee;
    ee.field_0 = 0;

    for (auto& e : entries)
    {
        ee.name = e.first;
        ee.offset = ToFilePos(e.second->second);
        sink.WriteGen(ee);
    }
}

void ExportsItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);

    for (auto& e : entries)
        os << e.first << " -> @" << e.second->first << '\n';
}

}
}
