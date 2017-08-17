#include "data.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"
#include <iostream>

namespace Neptools
{
namespace Stcm
{

void DataItem::Header::Validate(FilePosition chunk_size) const
{
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Stcm::DataItem::Header", x)
    VALIDATE(type < 0xff);
    VALIDATE(length <= chunk_size);
#undef VALIDATE
}

DataItem::DataItem(Key k, Context& ctx, const Header& raw, size_t chunk_size)
    : ItemWithChildren{k, ctx}
{
    raw.Validate(chunk_size);

    type = raw.type;
    offset_unit = raw.offset_unit;
    field_8 = raw.field_8;
}

DataItem& DataItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<Header>(ptr);

    auto& ret = x.ritem.SplitCreate<DataItem>(
        ptr.offset, x.t, x.ritem.GetSize() - ptr.offset - sizeof(Header));
    if (x.t.length > 0)
        ret.MoveNextToChild(x.t.length);

    LIBSHIT_ASSERT(ret.GetSize() == sizeof(Header) + x.t.length);

    // check heuristics
    if (!ret.GetChildren().empty())
        DataFactory::Check(ret);

    return ret;
}

void DataItem::Dump_(Sink& sink) const
{
    Header hdr;
    hdr.type = type;
    hdr.offset_unit = offset_unit;
    hdr.field_8 = field_8;
    hdr.length = GetSize() - sizeof(Header);
    sink.WriteGen(hdr);

    ItemWithChildren::Dump_(sink);
}

void DataItem::Inspect_(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);
    os << "data(" << type << ", " << offset_unit << ", " << field_8 << ')';
    InspectChildren(os, indent);
}

FilePosition DataItem::GetSize() const noexcept
{
    return ItemWithChildren::GetSize() + sizeof(Header);
}

void DataItem::Fixup()
{
    ItemWithChildren::Fixup_(sizeof(Header));
}

void DataFactory::Check(DataItem& it)
{
    for (auto f : GetStore())
        if (f(it)) return;
}

}
}

#include "data.binding.hpp"
