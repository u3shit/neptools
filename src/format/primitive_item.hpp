#ifndef UUID_2223739B_EEF5_4E62_B610_F34A86369503
#define UUID_2223739B_EEF5_4E62_B610_F34A86369503
#pragma once

#include "item.hpp"
#include "raw_item.hpp"
#include "sink.hpp"

namespace Neptools
{

  template <typename T, typename DumpT, typename Endian, char... Name>
  class PrimitiveItem final : public Item
  {
    static_assert(sizeof(T) == sizeof(DumpT));
    static_assert(sizeof(T) == sizeof(Endian));
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    using Type = T;

    PrimitiveItem(Key k, Context& ctx, T val)
      : Item{k, ctx}, value{val} {}
    PrimitiveItem(Key k, Context& ctx, Source src)
      : Item{k, ctx}
    {
      Union u;
      src.PreadGen<Libshit::Check::Throw>(0, u.dump);
      value = u.native;
    }
    static PrimitiveItem& CreateAndInsert(ItemPointer ptr)
    {
      auto x = RawItem::GetSource(ptr, -1);
      return x.ritem.SplitCreate<PrimitiveItem>(ptr.offset, x.src);
    }

    FilePosition GetSize() const noexcept override { return sizeof(T); }

    T value;

  private:
    union Union
    {
      T native;
      DumpT dump;
    };

    void Dump_(Sink& sink) const override
    {
      Union u{value};
      sink.WriteGen(Endian{u.dump});
    }
    void Inspect_(std::ostream& os, unsigned indent) const override
    {
      Item::Inspect_(os, indent);
      static constexpr const char name[] = { Name..., 0 };
      os << name << '(' << value << ')';
    }
  };

#define NEPTOOLS_PRIMITIVE_ITEMS(x)                                         \
  x(Int32Item, int32, PrimitiveItem<                                        \
    int32_t, int32_t, boost::endian::little_int32_t, 'i','n','t','3','2'>); \
  x(FloatItem, float, PrimitiveItem<                                        \
    float, int32_t, boost::endian::little_int32_t, 'f','l','o','a','t'>)
#define NEPTOOLS_GEN(cname, lname, ...) \
  using cname LIBSHIT_LUAGEN(fullname="neptools."..#lname.."_item") = __VA_ARGS__
  NEPTOOLS_PRIMITIVE_ITEMS(NEPTOOLS_GEN);
#undef NEPTOOLS_GEN

}

#endif
