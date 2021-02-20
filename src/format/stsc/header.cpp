#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

#include <libshit/char_utils.hpp>

namespace Neptools::Stsc
{

  void HeaderItem::Header::Validate(FilePosition size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Stsc::HeaderItem::Header", x)
    VALIDATE(memcmp(magic, "STSC", 4) == 0);
    VALIDATE(entry_point < size - 1);
    VALIDATE((flags & ~0x07) == 0);

    FilePosition hdr_len = sizeof(Header);
    if (flags & 1) hdr_len += 32;
    if (flags & 2) hdr_len += sizeof(ExtraHeader2Ser);
    if (flags & 4) hdr_len += 2; // uint16_t
    VALIDATE(entry_point >= hdr_len);
#undef VALIDATE
  }

  HeaderItem::HeaderItem(Key k, Context& ctx, Source src)
    : Item{k, ctx}, entry_point{Libshit::EmptyNotNull{}}
  {
    ADD_SOURCE(Parse_(ctx, src), src);
  }

  HeaderItem::HeaderItem(
      Key k, Context& ctx, Libshit::NotNull<LabelPtr> entry_point,
      std::optional<std::string_view> extra_headers_1,
      std::optional<ExtraHeaders2> extra_headers_2,
      std::optional<uint16_t> extra_headers_4)
    : Item{k, ctx}, entry_point{entry_point},
      extra_headers_2{extra_headers_2}, extra_headers_4{extra_headers_4}
  {
    if (extra_headers_1)
    {
      LIBSHIT_VALIDATE_FIELD("Ststc::HeaderItem", extra_headers_1->size() <= 32);
      this->extra_headers_1.emplace();

      auto s = extra_headers_1->size();
      memcpy(this->extra_headers_1->data(), extra_headers_1->data(), s);
      memset(this->extra_headers_1->data() + s, 0, 32 - s);
    }
  }

  FilePosition HeaderItem::GetSize() const noexcept
  {
    FilePosition size = sizeof(Header);
    if (extra_headers_1) size += 32;
    if (extra_headers_2) size += sizeof(ExtraHeader2Ser);
    if (extra_headers_4) size += 2;
    return size;
  }

  HeaderItem& HeaderItem::CreateAndInsert(ItemPointer ptr, Flavor flavor)
  {
    auto x = RawItem::GetSource(ptr, -1);
    auto& ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.src);

    InstructionBase::CreateAndInsert(ret.entry_point->GetPtr(), flavor);
    return ret;
  }

  void HeaderItem::Parse_(Context& ctx, Source& src)
  {
    src.CheckRemainingSize(sizeof(Header));
    auto hdr = src.ReadGen<Header>();
    hdr.Validate(src.GetSize());

    entry_point = ctx.CreateLabelFallback("entry_point", hdr.entry_point);

    if (hdr.flags & 1)
    {
      extra_headers_1.emplace();
      src.ReadGen(*extra_headers_1);
    }
    if (hdr.flags & 2)
    {
      auto eh2 = src.ReadGen<ExtraHeader2Ser>();
      extra_headers_2.emplace(
        eh2.field_0, eh2.field_2, eh2.field_4, eh2.field_6, eh2.field_8,
        eh2.field_a, eh2.field_c);
    }
    if (hdr.flags & 4)
      extra_headers_4 = src.ReadLittleUint16();
  }

  void HeaderItem::Dump_(Sink& sink) const
  {
    Header hdr;
    memcpy(hdr.magic, "STSC", 4);
    hdr.entry_point = ToFilePos(entry_point->GetPtr());
    uint32_t flags = 0;
    if (extra_headers_1) flags |= 1;
    if (extra_headers_2) flags |= 2;
    if (extra_headers_4) flags |= 4;
    hdr.flags = flags;
    sink.WriteGen(hdr);

    if (extra_headers_1) sink.WriteGen(*extra_headers_1);
    if (extra_headers_2)
    {
      ExtraHeader2Ser eh{
        extra_headers_2->field_0, extra_headers_2->field_2,
        extra_headers_2->field_4, extra_headers_2->field_6,
        extra_headers_2->field_8, extra_headers_2->field_a,
        extra_headers_2->field_c,
      };
      sink.WriteGen(eh);
    }
    if (extra_headers_4)
      sink.WriteLittleUint16(*extra_headers_4);
  }

  void HeaderItem::Inspect_(std::ostream& os, unsigned indent) const
  {
    Item::Inspect_(os, indent);

    os << "header(" << PrintLabel(entry_point) << ", ";
    if (extra_headers_1)
      Libshit::DumpBytes(os, {
          reinterpret_cast<const char*>(extra_headers_1->data()),
          extra_headers_1->size()});
    else os << "nil";

    if (extra_headers_2)
      os << ", neptools.stsc.header_item.extra_headers_2(" <<extra_headers_2->field_0
         << ", " << extra_headers_2->field_2
         << ", " << extra_headers_2->field_4
         << ", " << extra_headers_2->field_6
         << ", " << extra_headers_2->field_8
         << ", " << extra_headers_2->field_a
         << ", " << extra_headers_2->field_c << "), ";
    else os << ", nil, ";
    if (extra_headers_4) os << *extra_headers_4;
    else os << "nil";
    os << ")";
  }

}

#include "header.binding.hpp"
