#include "collection_link.hpp"
#include "../context.hpp"
#include "../cstring_item.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"
#include "../../container/vector.lua.hpp"
#include "../../sink.hpp"

namespace Neptools::Stcm
{

  void CollectionLinkHeaderItem::Header::Validate(FilePosition file_size) const
  {
#define VALIDATE(x)                                                     \
    LIBSHIT_VALIDATE_FIELD("Stcm::CollectionLinkHeaderItem::Header", x)

    VALIDATE(field_00 == 0);
    VALIDATE(offset <= file_size);
    VALIDATE(offset + sizeof(CollectionLinkItem::Entry)*count <= file_size);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
    VALIDATE(field_20 == 0 && field_24 == 0 && field_28 == 0 && field_2c == 0);
    VALIDATE(field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0);
#undef VALIDATE
  }

  void CollectionLinkItem::Entry::Validate(FilePosition file_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Stcm::CollectionLinkItem::Entry", x)
    VALIDATE(name_0 <= file_size);
    VALIDATE(name_1 <= file_size);
    VALIDATE(ptr == 0);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
#undef VALIDATE
  }

  CollectionLinkHeaderItem::CollectionLinkHeaderItem(
    Key k, Context& ctx, const Header& s)
    : Item{k, ctx},
      data{(s.Validate(ctx.GetSize()),
            ctx.CreateLabelFallback("collection_link", s.offset))}
  {}

  CollectionLinkHeaderItem& CollectionLinkHeaderItem::CreateAndInsert(
    ItemPointer ptr)
  {
    auto x = RawItem::Get<Header>(ptr);
    auto& ret = x.ritem.SplitCreate<CollectionLinkHeaderItem>(ptr.offset, x.t);

    auto ptr2 = ret.data->GetPtr();
    auto* ritem2 = ptr2.Maybe<RawItem>();
    if (!ritem2)
    {
      // HACK!
      LIBSHIT_VALIDATE_FIELD(
        "Stcm::CollectionLinkHeaderItem",
        ptr2.offset == 0 && x.t.count == 0);
      auto& eof = ptr2.AsChecked0<EofItem>();
      auto ctx = eof.GetContext();
      eof.Replace(ctx->Create<CollectionLinkItem>());
      return ret;
    }

    auto e = RawItem::GetSource(
      ptr2, x.t.count*sizeof(CollectionLinkItem::Entry));

    e.ritem.SplitCreate<CollectionLinkItem>(ptr2.offset, e.src, x.t.count);

    return ret;
  }

  void CollectionLinkHeaderItem::Dump_(Sink& sink) const
  {
    Header hdr{};
    hdr.offset = ToFilePos(data->GetPtr());
    hdr.count = data->GetPtr().As0<CollectionLinkItem>().entries.size();
    sink.WriteGen(hdr);
  }

  void CollectionLinkHeaderItem::Inspect_(
    std::ostream& os, unsigned indent) const
  {
    Item::Inspect_(os, indent);
    os << "collection_link_header(" << PrintLabel(data) << ")";
  }

  CollectionLinkItem::CollectionLinkItem(
    Key k, Context& ctx, Source src, uint32_t count)
    : Item{k, ctx}
  {
    ADD_SOURCE(Parse_(ctx, src, count), src);
  }

  void CollectionLinkItem::Dispose() noexcept
  {
    entries.clear();
    Item::Dispose();
  }

  void CollectionLinkItem::Parse_(Context& ctx, Source& src, uint32_t count)
  {
    entries.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
      auto e = src.ReadGen<Entry>();
      e.Validate(ctx.GetSize());

      auto& str0 = MaybeCreate<CStringItem>(ctx.GetPointer(e.name_0));
      auto& str1 = MaybeCreate<CStringItem>(ctx.GetPointer(e.name_1));
      entries.emplace_back(
        ctx.GetLabelTo(e.name_0, str0.GetLabelName()),
        ctx.GetLabelTo(e.name_1, str1.GetLabelName()));
    }
  }

  void CollectionLinkItem::Dump_(Sink& sink) const
  {
    Entry ee{};
    for (const auto& e : entries)
    {
      ee.name_0 = ToFilePos(e.name_0->GetPtr());
      ee.name_1 = ToFilePos(e.name_1->GetPtr());
      sink.WriteGen(ee);
    }
  }

  void CollectionLinkItem::Inspect_(std::ostream& os, unsigned indent) const
  {
    Item::Inspect_(os, indent);

    os << "collection_link{\n";
    for (const auto& e : entries)
    {
      Indent(os, indent+1) << '{' << PrintLabel(e.name_0) << ", "
                           << PrintLabel(e.name_1) << "},\n";
    }
    Indent(os, indent) << "}";
  }

}

NEPTOOLS_STD_VECTOR_LUAGEN(
  stcm_collection_link_item_link_entry,
  Neptools::Stcm::CollectionLinkItem::LinkEntry);
#include "collection_link.binding.hpp"
