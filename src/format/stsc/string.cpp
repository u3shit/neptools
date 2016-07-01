#include "string.hpp"
#include "../raw_item.hpp"

namespace Neptools
{
namespace Stsc
{

StringItem::StringItem(Key k, Context* ctx, Source src)
    : Item{k, ctx}, str{src.PreadCString(0)}
{}

StringItem* StringItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<StringItem>(ptr.offset, x.src);
}

void StringItem::Dump_(Sink& sink) const
{
    sink.WriteCString(str);
}

void StringItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);
    os << "string(";
    DumpBytes(os, str);
    os << ')';
}

}
}
