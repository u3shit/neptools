#ifndef UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#define UUID_C8313FD1_2CD9_4882_BDAD_E751A14AA2DF
#pragma once

#include "../item.hpp"
#include "../gbnl.hpp"
#include "file.hpp"

namespace Neptools { class RawItem; }

namespace Neptools::Stcm
{

  class GbnlItem final : public Item, public Gbnl
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    GbnlItem(Key k, Context& ctx, Source src)
      : Item{k, ctx}, Gbnl{std::move(src)}
    { PostCtor(ctx); }
    GbnlItem(Key k, Context& ctx, Endian endian, bool is_gstl, uint32_t flags,
             uint32_t field_28, uint32_t field_30,
             Libshit::AT<Gbnl::Struct::TypePtr> type)
      : Item{k, ctx},
        Gbnl{endian, is_gstl, flags, field_28, field_30, std::move(type)}
    { PostCtor(ctx); }
#if LIBSHIT_WITH_LUA
    GbnlItem(
      Key k, Context& ctx, Libshit::Lua::StateRef vm, Endian endian,
      bool is_gstl, uint32_t flags, uint32_t field_28, uint32_t field_30,
      Libshit::AT<Gbnl::Struct::TypePtr> type,
      Libshit::Lua::RawTable messages)
      : Item{k, ctx},
        Gbnl{vm, endian, is_gstl, flags, field_28, field_30, std::move(type),
             messages}
    { PostCtor(ctx); }
#endif

    void Dispose() noexcept override;

    static GbnlItem& CreateAndInsert(ItemPointer ptr);

    void Fixup() override { Gbnl::Fixup(); }
    FilePosition GetSize() const noexcept override { return Gbnl::GetSize(); }

  private:
    void PostCtor(Context& ctx) noexcept
    {
      if (auto file = dynamic_cast<File*>(&ctx))
        file->SetGbnl(*this);
    }

    void Dump_(Sink& sink) const override { Gbnl::Dump_(sink); }
    void Inspect_(std::ostream& os, unsigned indent) const override
    { Item::Inspect_(os, indent); Gbnl::InspectGbnl(os, indent); }
  };

  inline Libshit::Lua::DynamicObject& GetDynamicObject(GbnlItem& item)
  { return static_cast<Item&>(item); }

}
#endif
