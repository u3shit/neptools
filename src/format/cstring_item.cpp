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

std::string CStringItem::GetLabelName(std::string string)
{
    size_t iptr = 0, optr = 0;
    bool last_valid = false;
    for (size_t len = string.length(); iptr < len && optr < 16; ++iptr)
        if (isalnum(string[iptr]))
        {
            string[optr++] = string[iptr];
            last_valid = true;
        }
        else if (last_valid)
        {
            string[optr++] = '_';
            last_valid = false;
        }
    string.resize(optr);
    return "str_" + string;
}

}

#include "cstring_item.binding.hpp"
