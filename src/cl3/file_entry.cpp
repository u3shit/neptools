#include "file_entry.hpp"
#include "header.hpp"
#include "../context.hpp"

#include <boost/assert.hpp>

namespace Cl3
{

bool FileEntry::IsValid(size_t file_size) const noexcept
{
    return name.is_valid() &&
        field_200 == 0 &&
        data_offset + data_size <= file_size &&
        field_214 == 0 && field_218 == 0 && field_21c == 0 &&
        field_220 == 0 && field_224 == 0 && field_228 == 0 && field_22c == 0;
}

FileEntriesItem::FileEntriesItem(
    Key k, Context* ctx, const FileEntry* e, size_t count)
    : Item{k, ctx}
{
    entries.reserve(count);
    auto size = GetContext()->GetSize();

    for (size_t i = 0; i < count; ++i)
    {
        if (!e[i].IsValid(size))
            throw std::runtime_error("Invalid Cl3 FILE_COLLECTION header");

        entries.push_back({e[i].name, e[i].field_20c, e[i].field_210, nullptr});
    }
}

FileEntriesItem* FileEntriesItem::CreateAndInsert(RawItem* ritem, size_t cnt)
{
    auto s = reinterpret_cast<const FileEntry*>(ritem->GetPtr());

    if (ritem->GetSize() < cnt * sizeof(FileEntry))
        throw std::out_of_range("Cl3 FILE_COLLECTION too short");

    auto ret = ritem->Split(0, ritem->GetContext()->
        Create<FileEntriesItem>(s, cnt));

    for (size_t i = 0; i < cnt; ++i)
    {
        auto ptr2 = ret->GetContext()->GetPointer(
            ret->GetPosition() + s[i].data_offset);
        auto& ritem2 = dynamic_cast<RawItem&>(*ptr2.item);
        auto it2 = ritem2.Split(ptr2.offset, s[i].data_size);

        it2->InsertAfter(ret->GetContext()->
            Create<FileDataItem>(it2->GetPosition()));
        auto entr = it2->GetNext();
        entr->PrependChild(it2->Remove());
        ret->entries[i].data = ret->GetContext()->
            CreateLabelFallback(s[i].name.c_str(), {entr, 0});
    }
    return ret;
}

void FileEntriesItem::Dump(std::ostream& os) const
{
    FileEntry out{};

    for (const auto& e : entries)
    {
        out.name = e.name;
        BOOST_ASSERT(e.data->second.offset == 0);
        out.data_size = e.data->second.item->GetSize();
        out.data_offset = ToFilePos(e.data->second);

        os.write(reinterpret_cast<char*>(&out), sizeof(FileEntry));
    }
}

void FileEntriesItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    for (const auto& e : entries)
        os << "file(" << e.name << ", " << e.field_20c << ", " << e.field_210
           << ", @" << e.data->first << ")\n";
}

void FileDataItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "file_data {\n" << *GetChildren() << "\n}";
}

}
