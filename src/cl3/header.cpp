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
        secs_offset + num_sections * sizeof(Section) <= file_size &&
        field_14 == 1;
}

HeaderItem::HeaderItem(Key k, Context* ctx, const Header* raw)
    : Item{k, ctx}
{
    if (!raw->IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid Cl3 header");

    num_sections = raw->num_sections;
    sections = ctx->CreateLabelFallback("sections", raw->secs_offset);
}

HeaderItem* HeaderItem::CreateAndInsert(RawItem* ritem)
{
    if (ritem->GetSize() < sizeof(Header))
        throw std::out_of_range("Cl3 header too short");
    auto raw = reinterpret_cast<const Header*>(ritem->GetPtr());

    auto ret = ritem->Split(0, ritem->GetContext()->Create<HeaderItem>(raw));
    SectionsItem::CreateAndInsert(ret);
    return ret;
}

void HeaderItem::Dump(std::ostream& os) const
{
    Header hdr;
    memcpy(hdr.magic, "CL3L", 4);
    hdr.field_04 = 0;
    hdr.field_08 = 3;
    hdr.num_sections = num_sections;
    hdr.secs_offset = ToFilePos(sections->second);
    hdr.field_14 = 1;

    os.write(reinterpret_cast<char*>(&hdr), sizeof(Header));
}

void HeaderItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "cl3(" << num_sections << ", @" << sections->first << ')';
}

SectionsItem& HeaderItem::GetSectionsInt() const noexcept
{
    BOOST_ASSERT(sections->second.offset == 0 &&
                 dynamic_cast<SectionsItem*>(sections->second.item));
    return *static_cast<SectionsItem*>(sections->second.item);
}

}
