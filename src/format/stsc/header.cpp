#include "header.hpp"
#include "instruction.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

namespace Neptools
{
namespace Stsc
{

void HeaderItem::Header::Validate(FilePosition size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stsc::HeaderItem::Header", x)
    VALIDATE(memcmp(magic, "STSC", 4) == 0);
    VALIDATE(entry_point < size - 1);
    VALIDATE((flags & ~0x07) == 0);

    FilePosition hdr_len = sizeof(Header);
    if (flags & 1) hdr_len += 32;
    if (flags & 2) hdr_len += sizeof(ExtraHeader2);
    if (flags & 4) hdr_len += 2; // uint16_t
    VALIDATE(entry_point >= hdr_len);
#undef VALIDATE
}

HeaderItem::HeaderItem(Key k, Context& ctx, Source src)
    : Item{k, ctx}, entry_point{EmptyNotNull{}}
{
    AddInfo(&HeaderItem::Parse_, ADD_SOURCE(src), this, ctx, src);
}

FilePosition HeaderItem::GetSize() const noexcept
{
    FilePosition size = sizeof(Header);
    if (flags & 1) size += 32;
    if (flags & 2) size += sizeof(ExtraHeader2);
    if (flags & 4) size += 2;
    return size;
}

HeaderItem& HeaderItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    auto& ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.src);

    InstructionBase::CreateAndInsert(ret.entry_point->GetPtr());
    return ret;
}

void HeaderItem::Parse_(Context& ctx, Source& src)
{
    src.CheckRemainingSize(sizeof(Header));
    auto hdr = src.ReadGen<Header>();
    hdr.Validate(src.GetSize());

    entry_point = ctx.CreateLabelFallback("entry_point", hdr.entry_point);
    flags = hdr.flags;

    if (flags & 1)
        src.Read(extra_headers_1, 32);
    if (flags & 2)
    {
        auto eh2 = src.ReadGen<ExtraHeader2>();
        extra_headers_2_0 = eh2.field_0;
        extra_headers_2_2 = eh2.field_2;
        extra_headers_2_4 = eh2.field_4;
        extra_headers_2_6 = eh2.field_6;
        extra_headers_2_8 = eh2.field_8;
        extra_headers_2_a = eh2.field_a;
        extra_headers_2_c = eh2.field_c;
    }
    if (flags & 4)
        extra_headers_4 = src.ReadLittleUint16();
}

void HeaderItem::Dump_(Sink& sink) const
{
    Header hdr;
    memcpy(hdr.magic, "STSC", 4);
    hdr.entry_point = ToFilePos(entry_point->GetPtr());
    hdr.flags = flags;
    sink.WriteGen(hdr);

    if (flags & 1) sink.Write({extra_headers_1, 32});
    if (flags & 2)
    {
        ExtraHeader2 eh;
        eh.field_0 = extra_headers_2_0;
        eh.field_2 = extra_headers_2_2;
        eh.field_4 = extra_headers_2_4;
        eh.field_6 = extra_headers_2_6;
        eh.field_8 = extra_headers_2_8;
        eh.field_a = extra_headers_2_a;
        eh.field_c = extra_headers_2_c;
        sink.WriteGen(eh);
    }
    if (flags & 4)
        sink.WriteLittleUint16(extra_headers_4);
}

void HeaderItem::Inspect_(std::ostream& os) const
{
    Item::Inspect_(os);

    os << "stsc_header(@" << entry_point->GetName() << ", " << flags;
    if (flags & 1) { os << ", "; DumpBytes(os, {extra_headers_1, 32}); }
    if (flags & 2)
        os << ", " << extra_headers_2_0
           << ", " << extra_headers_2_2
           << ", " << extra_headers_2_4
           << ", " << extra_headers_2_6
           << ", " << extra_headers_2_8
           << ", " << extra_headers_2_a
           << ", " << extra_headers_2_c;
    if (flags & 4) os << ", " << extra_headers_4;
    os << ")\n";
}

}
}
