#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../container/vector.lua.hpp"

namespace Neptools::Stcm
{

GbnlItem& GbnlItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<GbnlItem>(ptr.offset, x.src);
}

}

NEPTOOLS_STD_VECTOR_FWD(gbnl_struct, Neptools::Gbnl::StructPtr);
#include "gbnl.binding.hpp"
