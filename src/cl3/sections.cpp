#include "sections.hpp"
#include "header.hpp"
#include "file_collection.hpp"
#include "../context.hpp"

#include <boost/assert.hpp>

namespace Cl3
{

bool Section::IsValid(size_t file_size) const noexcept
{
    return name.is_valid() &&
        data_offset + data_size <= file_size &&
        field_2c == 0 &&
        field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0 &&
        field_40 == 0 && field_44 == 0 && field_48 == 0 && field_4c == 0;
}

SectionsItem::SectionsItem(Key k, Context* ctx, const Section* s, size_t count)
    : Item{k, ctx}
{
    entries.reserve(count);
    auto size = GetContext()->GetSize();

    for (size_t i = 0; i < count; ++i)
    {
        if (!s[i].IsValid(size))
            throw std::runtime_error("Invalid Cl3 section");

        entries.push_back({s[i].name, s[i].count, nullptr});
    }
}

SectionsItem* SectionsItem::CreateAndInsert(const HeaderItem* hdr)
{
    auto& ptr = hdr->sections->second;
    auto& ritem = ptr.AsChecked<RawItem>();
    auto s = reinterpret_cast<const Section*>(ritem.GetPtr() + ptr.offset);

    uint32_t cnt = hdr->num_sections;
    if (ritem.GetSize() < cnt * sizeof(Section))
        throw std::out_of_range("Cl3 sections too short");

    auto ret = ritem.Split(ptr.offset, ritem.GetContext()->
        Create<SectionsItem>(s, cnt));

    for (size_t i = 0; i < cnt; ++i)
    {
        auto ptr2 = ret->GetContext()->GetPointer(s[i].data_offset);
        auto& ritem2 = ptr2.AsChecked<RawItem>();
        auto it2 = ritem2.Split(ptr2.offset, s[i].data_size);

        it2->InsertAfter(ret->GetContext()->
            Create<SectionEntryItem>(it2->GetPosition()));
        auto entr = it2->GetNext();
        entr->PrependChild(it2->Remove());
        ret->entries[i].data = ret->GetContext()->
            CreateLabelFallback(s[i].name.c_str(), {entr, 0});

        if (s[i].name == "FILE_COLLECTION")
            FileCollectionItem::CreateAndInsert(it2, s[i].count, ret);
    }
    return ret;
}

void SectionsItem::Dump(std::ostream& os) const
{
    Section sec{};

    for (const auto& e : entries)
    {
        sec.name = e.name;
        sec.count = e.count;
        BOOST_ASSERT(e.data->second.offset == 0);
        sec.data_size = e.data->second.item->GetSize();
        sec.data_offset = ToFilePos(e.data->second);

        os.write(reinterpret_cast<char*>(&sec), sizeof(Section));
    }
}

SectionEntryItem* SectionsItem::GetEntryInt(const char* name) const noexcept
{
    for (const auto& e : entries)
        if (e.name == name)
            return &e.data->second.As0<SectionEntryItem>();

    return nullptr;
}


void SectionsItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    for (const auto& e : entries)
        os << "section(" << e.name << ", " << e.count << ", @" << e.data->first
           << ")\n";
}

void SectionEntryItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "section_entry {\n" << *GetChildren() << "\n}";
}

}
