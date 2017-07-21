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
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    GbnlItem(Key k, Context& ctx, Source src)
        : Item{k, ctx}, Gbnl{std::move(src)} {}
    GbnlItem(Key k, Context& ctx, bool is_gstl, uint32_t flags,
             uint32_t field_28, uint32_t field_30,
             AT<Gbnl::Struct::TypePtr> type)
        : Item{k, ctx},
          Gbnl{is_gstl, flags, field_28, field_30, std::move(type)} {}
#ifndef NEPTOOLS_WITHOUT_LUA
    GbnlItem(
        Key k, Context& ctx, Lua::StateRef vm, bool is_gstl, uint32_t flags,
        uint32_t field_28, uint32_t field_30, AT<Gbnl::Struct::TypePtr> type,
        Lua::RawTable messages)
        : Item{k, ctx},
          Gbnl{vm, is_gstl, flags, field_28, field_30, std::move(type), messages} {}
#endif

    static GbnlItem& CreateAndInsert(ItemPointer ptr);

    void Fixup() override { Gbnl::Fixup(); }
    FilePosition GetSize() const noexcept override { return Gbnl::GetSize(); }

private:
    void Dump_(Sink& sink) const override { Gbnl::Dump_(sink); }
    void Inspect_(std::ostream& os, unsigned indent) const override
    { Item::Inspect_(os, indent); Gbnl::InspectGbnl(os, indent); }
};

}

namespace Lua
{
inline DynamicObject& GetDynamicObject(Stcm::GbnlItem& item)
{ return static_cast<Item&>(item); }
}

}
#endif
