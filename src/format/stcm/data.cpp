#include "data.hpp"
#include "gbnl.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include <iostream>
#include <boost/assert.hpp>

namespace Stcm
{

bool DataHeader::IsValid(size_t chunk_size) const noexcept
{
    return type < 0xff && length <= chunk_size;
}

DataItem::DataItem(Key k, Context* ctx, const DataHeader& raw, size_t chunk_size)
    : Item{k, ctx}
{
    if (!raw.IsValid(chunk_size))
        throw std::runtime_error("Invalid data header");

    type = raw.type;
    offset_unit = raw.offset_unit;
    field_8 = raw.field_8;
}

DataItem* DataItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<DataHeader>(ptr);

    auto ret = x.ritem.SplitCreate<DataItem>(
        ptr.offset, x.t, x.ritem.GetSize() - ptr.offset - sizeof(DataHeader));
    if (x.t.length > 0)
        ret->PrependChild(asserted_cast<RawItem*>(
            ret->GetNext())->Split(0, x.t.length)->Remove());
    BOOST_ASSERT(ret->GetSize() == sizeof(DataHeader) + x.t.length);

    // hack
    auto child = dynamic_cast<RawItem*>(ret->GetChildren());
    if (child && child->GetSize() > sizeof(GbnlFooter))
    {
        char buf[4];
        child->GetSource().Pread(child->GetSize() - sizeof(GbnlFooter), buf, 4);
        if (memcmp(buf, "GBNL", 4) == 0)
            GbnlItem::CreateAndInsert({child, 0});
    }

    return ret;
}

void DataItem::Dump(std::ostream& os) const
{
    DataHeader hdr;
    hdr.type = type;
    hdr.offset_unit = offset_unit;
    hdr.field_8 = field_8;
    hdr.length = GetSize() - sizeof(DataHeader);
    os.write(reinterpret_cast<char*>(&hdr), sizeof(DataHeader));

    for (auto it = GetChildren(); it; it = it->GetNext())
        it->Dump(os);
}

void DataItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);
    os << "data(" << type << ", " << offset_unit << ", " << field_8 << ") {";
    if (GetChildren()) os << '\n' << *GetChildren();
    os << '}';
}

size_t DataItem::GetSize() const noexcept
{
    size_t ret = sizeof(DataHeader);
    for (auto it = GetChildren(); it; it = it->GetNext())
        ret += it->GetSize();
    return ret;
}

size_t DataItem::UpdatePositions(FilePosition npos)
{
    if (GetChildren())
        GetChildren()->UpdatePositions(
            npos + sizeof(DataHeader));
    return Item::UpdatePositions(npos);
}

}
