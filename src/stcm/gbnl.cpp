#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"

namespace Stcm
{

GbnlItem::GbnlItem(Key k, Context* ctx, const Byte* data, size_t len)
    : Item{k, ctx}, Gbnl{data, len}
{
}

GbnlItem* GbnlItem::CreateAndInsert(RawItem* ritem)
{
    auto nitem = ritem->GetContext()->Create<GbnlItem>(
        ritem->GetPtr(), ritem->GetSize());
    auto ret = nitem.get();
    ritem->Replace(std::move(nitem));
    return ret;
}

}
