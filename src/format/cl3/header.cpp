#include "header.hpp"
#include "sections.hpp"
#include "../context.hpp"
#include <boost/assert.hpp>

namespace Cl3
{

bool Header::IsValid(size_t file_size) const noexcept
{
    return memcmp(magic, "CL3L", 4) == 0 &&
        field_04 == 0 &&
        field_08 == 3 &&
        secs_offset + num_sections * sizeof(Section) <= file_size;
}

HeaderItem::HeaderItem(Key k, Context* ctx, const Header* raw)
    : Item{k, ctx}
{
    if (!raw->IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid Cl3 header");

    field_14 = raw->field_14;
    sections = ctx->CreateLabelFallback("sections", raw->secs_offset);
}

HeaderItem* HeaderItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::Get<Header>(ptr);
    size_t num_secs = x.ptr->num_sections;
    if (x.len < sizeof(Header))
        throw std::out_of_range("Cl3 header too short");

    auto ret = x.ritem.SplitCreate<HeaderItem>(ptr.offset, x.ptr);
    SectionsItem::CreateAndInsert(ret->sections->second, num_secs);
    return ret;
}

void HeaderItem::Dump(std::ostream& os) const
{
    Header hdr;
    memcpy(hdr.magic, "CL3L", 4);
    hdr.field_04 = 0;
    hdr.field_08 = 3;
    hdr.num_sections = sections->second.AsChecked0<SectionsItem>().entries.size();
    hdr.secs_offset = ToFilePos(sections->second);
    hdr.field_14 = field_14;

    os.write(reinterpret_cast<char*>(&hdr), sizeof(Header));
}

void HeaderItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "cl3(@" << sections->first << ", " << field_14 << ')';
}

SectionsItem& HeaderItem::GetSectionsInt() const noexcept
{
    return sections->second.As0<SectionsItem>();
}

}
