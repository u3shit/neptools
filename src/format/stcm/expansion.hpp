#ifndef GUARD_KNOBBILY_BROGUED_NET_REGISTER_TON_BESOTS_7814
#define GUARD_KNOBBILY_BROGUED_NET_REGISTER_TON_BESOTS_7814
#pragma once

#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools::Stcm
{

  class ExpansionItem final : public Item
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct Header
    {
      boost::endian::little_uint32_t index;
      boost::endian::little_uint32_t name;
      boost::endian::little_uint64_t ptr; // used by game engine
      boost::endian::little_uint32_t pad[16];

      void Validate(FilePosition file_size) const;
    };
    static_assert(sizeof(Header) == 0x50);

    ExpansionItem(Key k, Context& ctx, uint32_t index, LabelPtr name)
      : Item{k, ctx}, index{index}, name{name} {}
    LIBSHIT_NOLUA
    ExpansionItem(Key k, Context& ctx, const Header& hdr);
    static ExpansionItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override { return sizeof(Header); }

    uint32_t index;
    Libshit::NotNull<LabelPtr> name;

  private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
  };

  class ExpansionsItem final : public ItemWithChildren
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    ExpansionsItem(Key k, Context& ctx) : ItemWithChildren{k, ctx} {}
    static ExpansionsItem& CreateAndInsert(ItemPointer ptr, uint32_t count);

  private:
    void Inspect_(std::ostream& os, unsigned indent) const override;
  };

}
#endif
