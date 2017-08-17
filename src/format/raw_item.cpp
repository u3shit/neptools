#include "raw_item.hpp"
#include "context.hpp"
#include <iomanip>

namespace Neptools
{

void RawItem::Dump_(Sink& sink) const
{
    src.Dump(sink);
}

void RawItem::Inspect_(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);
    os << "raw(" << Quoted(src) << ")";
}

Libshit::NotNull<Libshit::RefCountedPtr<RawItem>> RawItem::InternalSlice(
    FilePosition spos, FilePosition slen)
{
    LIBSHIT_ASSERT(spos+slen <= GetSize());
    auto ctx = GetContext();
    return ctx->Create<RawItem>(Source{src, spos, slen}, position+spos);
}

// split into 3 parts: 0...pos, pos...pos+nitem size, pos+nitem size...this size
void RawItem::Split2(
    FilePosition pos, Libshit::NotNull<Libshit::SmartPtr<Item>> nitem)
{
    auto len = nitem->GetSize();
    LIBSHIT_ASSERT(pos <= GetSize() && pos+len <= GetSize());
    auto rem_len = GetSize() - len - pos;

    if (pos == 0 && rem_len == 0)
    {
        Replace(std::move(nitem));
        return;
    }

    SliceSeq seq;
    if (pos != 0) seq.emplace_back(MakeNotNull(this), pos);
    seq.emplace_back(std::move(nitem), pos+len);
    if (rem_len > 0)
        if (pos == 0)
            seq.emplace_back(MakeNotNull(this), GetSize());
        else
            seq.emplace_back(InternalSlice(pos+len, rem_len), GetSize());

    Item::Slice(std::move(seq));
    if (pos == 0)
    {
        LIBSHIT_ASSERT(rem_len > 0);
        src.Slice(len, rem_len);
    }
    else
        src.Slice(0, pos);
}

RawItem& RawItem::Split(FilePosition offset, FilePosition size)
{
    auto it = InternalSlice(offset, size);
    auto& ret = *it;
    Split2(offset, std::move(it));
    return ret;
}

}

#include "raw_item.binding.hpp"
