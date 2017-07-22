#include "cstring_item.hpp"
#include "raw_item.hpp"
#include "../sink.hpp"

namespace Neptools
{

CStringItem::CStringItem(Key k, Context& ctx, const Source& src)
    : Item{k, ctx}, string{src.PreadCString(0)}
{}

CStringItem& CStringItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<CStringItem>(ptr.offset, x.src);
}

void CStringItem::Dump_(Sink& sink) const
{
    sink.WriteCString(string);
}

void CStringItem::Inspect_(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);
    os << "c_string(" << Quoted(string) << ')';
}

}

#include "cstring_item.binding.hpp"
