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

    // complete replace
    if (pos == 0 && rem_len == 0)
    {
        Replace(std::move(nitem)); // moves labels too
    }
    // no previous element
    else if (pos == 0)
    {
        LabelsContainer cthis, cnitem;
        for (auto it : GetLabels())
            if (it.first >= len)
                cthis.insert({it.first-len, it.second});
            else
                cnitem.insert({it.first, it.second});

        CommitLabels({}, std::move(cthis));
        nitem->CommitLabels({}, std::move(cnitem));

        InsertBefore(std::move(nitem));
        this->offset += len;
        this->len -= len;
        this->position += len;
    }
    else
    {
        // filter out new labels
        LabelsContainer cnitem, cnext;
        for (auto it : GetLabels())
            if (it.first >= pos+len)
                cnext.insert({it.first-pos-len, it.second});
            else if (it.first >= pos)
                cnitem.insert({it.first-pos, it.second});

        // if we need the third element, alloc it here
        if (rem_len != 0)
        {
            std::unique_ptr<RawItem> split{new RawItem(
                GetContext(), buf, offset+pos+len, rem_len, position+pos+len)};
            // everything is noexcept after this
            split->CommitLabels({}, std::move(cnext));
            nitem->InsertAfter(std::move(split));
        }
        else
            BOOST_ASSERT(cnext.empty());

        nitem->CommitLabels({}, std::move(cnitem));
        InsertAfter(std::move(nitem));

        TrimLabels(pos);
        this->len = pos;
    }
}

namespace
{
struct PmapRem
{
    using T = std::pair<PointerMap::iterator, bool>;
    PointerMap& pmap;
    T x;
    ~PmapRem() { if (x.second) pmap.erase(x.first); }
};
}

void RawItem::Split(size_t pos, std::unique_ptr<Item> nitem, PointerMap& pmap)
{
    auto sav = nitem.get();
    // prealloc new PointerMap items, so in case of an exception we can rollback
    PmapRem p0{pmap, pmap.insert({sav->GetPosition() + pos, {}})};
    PmapRem p1{pmap, pmap.insert({sav->GetPosition() + sav->GetSize() + pos, {}})};

    Split(pos, std::move(nitem));

    // actually set PointerMap
    if (auto ptr = sav->GetPrev())
    {
        auto p = pmap.find(ptr->GetPosition());
        BOOST_ASSERT(p != pmap.end());
        p->second = ptr;
    }

    p0.x.first->second = sav;
    p0.x.second = false;

    if (auto ptr = sav->GetNext())
    {
        BOOST_ASSERT(p1.x.first == pmap.find(ptr->GetPosition()));
        p1.x.first->second = ptr;
        p1.x.second = false;
    }
}
