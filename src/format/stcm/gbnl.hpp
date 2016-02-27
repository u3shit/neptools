#ifndef UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#define UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#pragma once

#include "../item.hpp"
#include "../gbnl.hpp"

class RawItem;

namespace Stcm
{

class GbnlItem final : public Item, public Gbnl
{
public:
    GbnlItem(Key k, Context* ctx, Source src);
    static GbnlItem* CreateAndInsert(ItemPointer ptr);

    void Dump(std::ostream& os) const override { Gbnl::Dump(os); }
    void PrettyPrint(std::ostream& os) const override { Gbnl::Inspect(os); }
    size_t GetSize() const noexcept override { return Gbnl::GetSize(); }
};

}

#endif
