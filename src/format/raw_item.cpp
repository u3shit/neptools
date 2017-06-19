#include "raw_item.hpp"
#include "context.hpp"
#include <iomanip>

namespace Neptools
{

static inline char FilterPrintable(Byte c)
{
    //return isprint(c) ? c : '.';
    if (c >= ' ' && c < '~')
        return c;
    else
        return '.';
}

void RawItem::Dump_(Sink& sink) const
{
    src.Dump(sink);
}

void RawItem::Inspect_(std::ostream& os) const
{
    auto flags = os.flags();
    os << std::hex;

    auto it = GetLabels().begin();
    size_t i = 0;
    while (true)
    {
        for (; it != GetLabels().end() && it->GetPtr().offset == i; ++it)
            os << '@' << it->GetName() << ":\n";
        auto max = GetSize();
        if (it != GetLabels().end() && it->GetPtr().offset < max)
            max = it->GetPtr().offset;

        os << std::setw(8) << std::setfill('0') << GetPosition() + i
           << ' ';

        Byte buf[16];
        src.Pread(i, buf, std::min<uint64_t>(max-i, 16));

        // numbers
        size_t j = 0;
        for (; j < 8 && i+j < max; ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>(buf[j]);
        os << ' ';
        for (; j < 16 && i+j < max; ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>(buf[j]);
        for (; j < 16; ++j) os << "   ";

        os << " |";
        // chars
        j = 0;
        for (; j < 16 && i+j < max; ++j)
            os << FilterPrintable(buf[j]);
        os << '|';
        if ((i+=j) >= GetSize()) break;
        os << '\n';
    }
    NEPTOOLS_ASSERT(it == GetLabels().end());
    os.flags(flags);
}

NotNull<RefCountedPtr<RawItem>> RawItem::InternalSlice(
    FilePosition spos, FilePosition slen)
{
    NEPTOOLS_ASSERT(spos+slen <= GetSize());
    auto ctx = GetContext();
    return ctx->Create<RawItem>(Source{src, spos, slen}, position+spos);
}

// split into 3 parts: 0...pos, pos...pos+nitem size, pos+nitem size...this size
void RawItem::Split2(FilePosition pos, NotNull<SmartPtr<Item>> nitem)
{
    auto len = nitem->GetSize();
    NEPTOOLS_ASSERT(pos <= GetSize() && pos+len <= GetSize());
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
        NEPTOOLS_ASSERT(rem_len > 0);
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
