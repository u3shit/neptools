#include "raw_item.hpp"
#include <iomanip>
#include <boost/assert.hpp>

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
    auto flags = os.flags();
    os << std::hex;


    size_t i = 0;
    do
    {
        os << std::setw(8) << std::setfill('0') << GetPosition() + i
           << ' ';

        // numbers
        size_t j = 0;
        for (; j < 8 && i+j < GetSize(); ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>((*this)[i+j]);
        os << ' ';
        for (; j < 16 && i+j < GetSize(); ++j)
            os << ' ' << std::setw(2) << static_cast<unsigned>((*this)[i+j]);
        for (; j < 16; ++j) os << "   ";

        os << " |";
        // chars
        j = 0;
        for (; j < 16 && i+j < GetSize(); ++j)
            os << FilterPrintable((*this)[i+j]);
        os << "|\n";
    }
    while ((i += 16) < GetSize());
    os.flags(flags);
}

// split into 3 parts: 0...pos, pos...pos+nitem size, pos+nitem size...this size
void RawItem::Split(size_t pos, std::unique_ptr<Item> nitem)
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
        {
            std::unique_ptr<RawItem> split{new RawItem(
                GetContext(), buf, offset+pos+len, rem_len, position+pos+len)};
            seq.push_back({std::move(split), pos+len});
        }

    Slice(std::move(seq));
    if (pos == 0 && rem_len > 0)
    {
        this->offset += len;
        this->len -= len;
    }
}
