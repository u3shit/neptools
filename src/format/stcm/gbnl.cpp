#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"

namespace Neptools
{
namespace Stcm
{

GbnlItem::GbnlItem(Key k, Context* ctx, Source src)
    : Item{k, ctx}, Gbnl{std::move(src)}
{
}

GbnlItem& GbnlItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    return x.ritem.SplitCreate<GbnlItem>(ptr.offset, x.src);
}

}
}

#include "gbnl.binding.hpp"
