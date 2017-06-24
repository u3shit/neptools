#include "string.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

namespace Neptools
{
namespace Stsc
{

StringItem::StringItem(Key k, Context& ctx, const Source& src)
    : Item{k, ctx}, string{src.PreadCString(0)}
{}

StringItem& StringItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<StringItem>(ptr.offset, x.src);
}

void StringItem::Dump_(Sink& sink) const
{
    sink.WriteCString(string);
}

void StringItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);
    os << "string(";
    DumpBytes(os, string);
    os << ')';
}

}
}

#include "string.binding.hpp"
