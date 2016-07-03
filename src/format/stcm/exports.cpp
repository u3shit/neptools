#include "exports.hpp"
#include "data.hpp"
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
    VALIDATE(type == Type::CODE || type == Type::DATA);
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
    auto size = GetContext().GetSize();
    for (uint32_t i = 0; i < export_count; ++i)
    {
        auto e = src.ReadGen<Entry>();
        e.Validate(size);
        entries.push_back({
            static_cast<Type>(static_cast<uint32_t>(e.type)),
            e.name,
            &GetContext().CreateLabelFallback(e.name.c_str(), e.offset)});
    }
}

ExportsItem& ExportsItem::CreateAndInsert(ItemPointer ptr, uint32_t export_count)
{
    auto x = RawItem::GetSource(ptr, export_count*sizeof(Entry));

    auto& ret = x.ritem.SplitCreate<ExportsItem>(
        ptr.offset, x.src, export_count);

    for (const auto& e : ret.entries)
        switch (e.type)
        {
        case Type::CODE:
            MaybeCreate<InstructionItem>(e.lbl->ptr);
            break;
        case Type::DATA:
            MaybeCreate<DataItem>(e.lbl->ptr);
            break;
        }
    return ret;
}

void ExportsItem::Dump_(Sink& sink) const
{
    Entry ee;

    for (auto& e : entries)
    {
        ee.type = e.type;
        ee.name = e.name;
        ee.offset = ToFilePos(e.lbl->ptr);
        sink.WriteGen(ee);
    }
}

void ExportsItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);

    for (auto& e : entries)
    {
        os << '{' << e.type << ", ";
        DumpBytes(os, e.name.c_str());
        os << ", @" << e.lbl->name << ")\n";
    }
}

}
}
