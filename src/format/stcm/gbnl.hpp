#ifndef UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#define UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#pragma once

#include "../item.hpp"
#include "../gbnl.hpp"

namespace Neptools
{
class RawItem;

namespace Stcm
{

class GbnlItem final : public Item, public Gbnl
{
public:
    GbnlItem(Key k, Context* ctx, Source src);
    static GbnlItem& CreateAndInsert(ItemPointer ptr);

    void Fixup() override { Gbnl::Fixup(); }
    FilePosition GetSize() const noexcept override { return Gbnl::GetSize(); }

private:
    void Dump_(Sink& sink) const override { Gbnl::Dump_(sink); }
    void Inspect_(std::ostream& os) const override { Gbnl::Inspect_(os); }
};

// HACK!!!
inline void intrusive_ptr_add_ref(const GbnlItem* it)
{ intrusive_ptr_add_ref(static_cast<const Item*>(it)); }
inline void intrusive_ptr_release(const GbnlItem* it)
{ intrusive_ptr_release(static_cast<const Item*>(it)); }

}
}
#endif
