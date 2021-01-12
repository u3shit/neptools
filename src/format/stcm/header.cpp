#include "header.hpp"
#include "collection_link.hpp"
#include "expansion.hpp"
#include "exports.hpp"
#include "../context.hpp"
#include "../../sink.hpp"
#include "../../utils.hpp"

#include <libshit/char_utils.hpp>
#include <stdexcept>
#include <iostream>

namespace Neptools::Stcm
{

  void HeaderItem::Header::Validate(FilePosition file_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Stcm::HeaderItem::Header", x)
    VALIDATE(memcmp(magic, "STCM2", 5) == 0);
    VALIDATE(endian == 'L');
    VALIDATE(msg.is_valid());
    VALIDATE(export_offset < file_size - 0x28*export_count);
    VALIDATE(collection_link_offset < file_size);
    VALIDATE(field_30 == 0);
    VALIDATE(expansion_count == 0 || expansion_offset != 0);
    VALIDATE(expansion_offset < file_size - 0x50*expansion_count);
#undef VALIDATE
  }

  HeaderItem::HeaderItem(Key k, Context& ctx, const Header& hdr)
    : Item{k, ctx}, export_sec{Libshit::EmptyNotNull{}},
      collection_link{Libshit::EmptyNotNull{}}
  {
    hdr.Validate(ctx.GetSize());

    msg = hdr.msg;
    export_sec = ctx.CreateLabelFallback("exports", hdr.export_offset);
    collection_link = ctx.CreateLabelFallback(
      "collection_link_hdr", hdr.collection_link_offset);;
    if (hdr.expansion_offset != 0)
      expansion = ctx.CreateLabelFallback("expansion", hdr.expansion_offset);
    field_28 = hdr.field_28;
  }

  HeaderItem& HeaderItem::CreateAndInsert(ItemPointer ptr)
  {
    auto x = RawItem::Get<Header>(ptr);

    auto& ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.t);
    CollectionLinkHeaderItem::CreateAndInsert(ret.collection_link->GetPtr());
    if (ret.expansion)
      ExpansionsItem::CreateAndInsert(ret.expansion->GetPtr(),
                                      x.t.expansion_count);
    ExportsItem::CreateAndInsert(ret.export_sec->GetPtr(), x.t.export_count);
    return ret;
  }

  void HeaderItem::Dump_(Sink& sink) const
  {
    Header hdr{};
    memcpy(hdr.magic, "STCM2", 5);
    hdr.endian = 'L';
    hdr.msg = msg;
    hdr.export_offset = ToFilePos(export_sec->GetPtr());
    hdr.export_count = export_sec->GetPtr().As0<ExportsItem>().entries.size();
    hdr.field_28 = field_28;
    hdr.collection_link_offset = ToFilePos(collection_link->GetPtr());
    if (expansion)
    {
      hdr.expansion_offset = ToFilePos(expansion->GetPtr());
      hdr.expansion_count = expansion->GetPtr().As0<ExpansionsItem>().
        GetChildren().size();
    }

    sink.WriteGen(hdr);
  }

  void HeaderItem::Inspect_(std::ostream& os, unsigned indent) const
  {
    Item::Inspect_(os, indent);

    os << "header(" << Libshit::Quoted(msg.c_str()) << ", "
       << PrintLabel(export_sec) << ", " << PrintLabel(collection_link) << ", "
       << field_28 << ", " << PrintLabel(expansion) << ")";
  }

}

#include "header.binding.hpp"
