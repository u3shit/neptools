#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"

namespace Neptools::Stcm
{

GbnlItem& GbnlItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<GbnlItem>(ptr.offset, x.src);
}

}

#include "gbnl.binding.hpp"
