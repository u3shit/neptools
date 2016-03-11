#include "raw_item.hpp"
#include "context.hpp"
#include <iomanip>

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
        for (; it != GetLabels().end() && it->first == i; ++it)
            os << '@' << it->second->first << ":\n";
        auto max = GetSize();
        if (it != GetLabels().end() && it->first < max)
            max = it->first;

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
    BOOST_ASSERT(it == GetLabels().end());
    os.flags(flags);
}

std::unique_ptr<RawItem> RawItem::InternalSlice(
    FilePosition spos, FilePosition slen)
{
    BOOST_ASSERT(spos+slen <= GetSize());
    return GetContext()->Create<RawItem>(Source{src, spos, slen}, position+spos);
}

// split into 3 parts: 0...pos, pos...pos+nitem size, pos+nitem size...this size
void RawItem::Split2(FilePosition pos, std::unique_ptr<Item> nitem)
{
    auto len = nitem->GetSize();
    BOOST_ASSERT(pos <= GetSize() && pos+len <= GetSize());
    auto rem_len = GetSize() - len - pos;

    if (pos == 0 && rem_len == 0)
    {
        Replace(std::move(nitem));
        return;
    }

    SliceSeq seq;
    if (pos != 0) seq.push_back({nullptr, 0});
    seq.push_back({std::move(nitem), pos});
    if (rem_len > 0)
        if (pos == 0)
            seq.push_back({nullptr, len});
        else
            seq.push_back({InternalSlice(pos+len, rem_len), pos+len});

    Item::Slice(std::move(seq));
    if (pos == 0)
    {
        BOOST_ASSERT(rem_len > 0);
        src.Slice(len, rem_len);
    }
    else
        src.Slice(0, pos);
}

RawItem* RawItem::Split(FilePosition offset, FilePosition size)
{
    auto it = InternalSlice(offset, size);
    auto ret = it.get();
    Split2(offset, std::move(it));
    return ret;
}
