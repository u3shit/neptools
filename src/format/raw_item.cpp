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

void RawItem::Dump(std::ostream& os) const
{
    os.write(reinterpret_cast<const char*>(GetPtr()), GetSize());
}

void RawItem::PrettyPrint(std::ostream& os) const
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

        // numbers
        size_t j = 0;
        for (; j < 8 && i+j < max; ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>((*this)[i+j]);
        os << ' ';
        for (; j < 16 && i+j < max; ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>((*this)[i+j]);
        for (; j < 16; ++j) os << "   ";

        os << " |";
        // chars
        j = 0;
        for (; j < 16 && i+j < max; ++j)
            os << FilterPrintable((*this)[i+j]);
        os << '|';
        if ((i+=j) >= GetSize()) break;
        os << '\n';
    }
    BOOST_ASSERT(it == GetLabels().end());
    os.flags(flags);
}

std::unique_ptr<RawItem> RawItem::InternalSlice(size_t spos, size_t slen)
{
    BOOST_ASSERT(spos+slen <= len);
    return GetContext()->Create<RawItem>(buf, offset+spos, slen, position+spos);
}

// split into 3 parts: 0...pos, pos...pos+nitem size, pos+nitem size...this size
void RawItem::Split2(size_t pos, std::unique_ptr<Item> nitem)
{
    size_t len = nitem->GetSize();
    BOOST_ASSERT(pos <= GetSize() && pos+len <= GetSize());
    size_t rem_len = GetSize() - len - pos;

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
        this->offset += len;
        this->len = rem_len;
    }
    else
        this->len = pos;
}

RawItem* RawItem::Split(size_t offset, size_t size)
{
    auto it = InternalSlice(offset, size);
    auto ret = it.get();
    Split2(offset, std::move(it));
    return ret;
}
