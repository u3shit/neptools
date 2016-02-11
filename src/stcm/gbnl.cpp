#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"

namespace Stcm
{

GbnlItem::GbnlItem(Key k, Context* ctx, const Byte* data, size_t len)
    : Item{k, ctx}, Gbnl{data, len}
{
}

GbnlItem* GbnlItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<Byte>(ptr);
    return x.ritem.SplitCreate<GbnlItem>(ptr.offset, x.ptr, x.len);
}

}
