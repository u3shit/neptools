#include "exports.hpp"
#include "data.hpp"
#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include "../../sink.hpp"
#include "../../container/vector.lua.hpp"
#include <iostream>

namespace Neptools
{
namespace Stcm
{

void ExportsItem::Entry::Validate(FilePosition file_size) const
{
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Stcm::ExportsItem::Entry", x)
    VALIDATE(type == Type::CODE || type == Type::DATA);
    VALIDATE(name.is_valid());
    VALIDATE(offset < file_size);
#undef VALIDATE
}

ExportsItem::ExportsItem(Key k, Context& ctx, Source src, uint32_t export_count)
    : Item{k, ctx}
{
    AddInfo(&ExportsItem::Parse_, ADD_SOURCE(src), this, ctx, src, export_count);
}

void ExportsItem::Parse_(Context& ctx, Source& src, uint32_t export_count)
{
    entries.reserve(export_count);
    auto size = ctx.GetSize();
    for (uint32_t i = 0; i < export_count; ++i)
    {
        auto e = src.ReadGen<Entry>();
        e.Validate(size);
        entries.push_back(Libshit::MakeSmart<EntryType>(
            static_cast<Type>(static_cast<uint32_t>(e.type)),
            e.name,
            ctx.CreateLabelFallback(e.name.c_str(), e.offset)));
    }
}

ExportsItem& ExportsItem::CreateAndInsert(ItemPointer ptr, uint32_t export_count)
{
    auto x = RawItem::GetSource(ptr, export_count*sizeof(Entry));

    auto& ret = x.ritem.SplitCreate<ExportsItem>(
        ptr.offset, x.src, export_count);

    for (const auto& e : ret.entries)
        switch (e->type)
        {
        case Type::CODE:
            MaybeCreate<InstructionItem>(e->lbl->GetPtr());
            break;
        case Type::DATA:
            MaybeCreate<DataItem>(e->lbl->GetPtr());
            break;
        }
    return ret;
}

void ExportsItem::Dispose() noexcept
{
    entries.clear();
    Item::Dispose();
}

void ExportsItem::Dump_(Sink& sink) const
{
    Entry ee;

    for (auto& e : entries)
    {
        ee.type = e->type;
        ee.name = e->name;
        ee.offset = ToFilePos(e->lbl->GetPtr());
        sink.WriteGen(ee);
    }
}

void ExportsItem::Inspect_(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);

    os << "exports{\n";
    for (auto& e : entries)
    {
        Indent(os, indent+1) << '{' << e->type << ", " <<
            Quoted(e->name.c_str()) << ", " << PrintLabel(e->lbl) << "},\n";
    }
    Indent(os, indent) << '}';
}

}
}

NEPTOOLS_STD_VECTOR_LUAGEN(
    stcm_exports_item_entry_type, Libshit::NotNull<Libshit::RefCountedPtr<
        Neptools::Stcm::ExportsItem::EntryType>>);
#include "exports.binding.hpp"
