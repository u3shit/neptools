#include "file.hpp"
#include "file_collection.hpp"
#include "sections.hpp"
#include "header.hpp"
#include "../context.hpp"

#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

namespace Cl3
{

bool FileEntry::IsValid(size_t rem_size) const noexcept
{
    return name.is_valid() &&
        field_200 == 0 &&
        data_offset + data_size <= rem_size &&
        field_214 == 0 && field_218 == 0 && field_21c == 0 &&
        field_220 == 0 && field_224 == 0 && field_228 == 0 && field_22c == 0;
}

FileCollectionItem::FileCollectionItem(
    Key k, Context* ctx, FilePosition pos, const FileEntry* e, size_t count,
    SectionsItem* secs)
    : Item{k, ctx, pos}, secs{secs}
{
    entries.reserve(count);
    auto size = GetContext()->GetSize();

    for (size_t i = 0; i < count; ++i)
    {
        if (!e[i].IsValid(size - GetPosition()))
            throw std::runtime_error("Invalid Cl3 FILE_COLLECTION header");

        entries.push_back({e[i].name, e[i].field_20c, e[i].field_210, nullptr});
    }
}

FileCollectionItem* FileCollectionItem::CreateAndInsert(
    RawItem* ritem, size_t cnt, SectionsItem* secs)
{
    auto s = reinterpret_cast<const FileEntry*>(ritem->GetPtr());

    if (ritem->GetSize() < cnt * sizeof(FileEntry))
        throw std::out_of_range("Cl3 FILE_COLLECTION too short");

    auto ret = ritem->Split(0, ritem->GetContext()->
        Create<FileCollectionItem>(ritem->GetPosition(), s, cnt, secs));

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

void FileCollectionItem::Dump(std::ostream& os) const
{
    FileEntry out{};

    for (const auto& e : entries)
    {
        out.name = e.name;
        BOOST_ASSERT(e.data->second.offset == 0);
        out.data_size = e.data->second.item->GetSize();
        out.data_offset = ToFilePos(e.data->second) - GetPosition();
        out.field_20c = e.field_20c;
        out.field_210 = e.field_210;

        os.write(reinterpret_cast<char*>(&out), sizeof(FileEntry));
    }
}

void FileCollectionItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    for (const auto& e : entries)
        os << "file(" << e.name << ", " << e.field_20c << ", " << e.field_210
           << ", @" << e.data->first << ")\n";
}

FileDataItem* FileCollectionItem::GetFileInt(const char* name) const noexcept
{
    for (const auto& e : entries)
        if (e.name == name)
        {
            BOOST_ASSERT(e.data->second.offset == 0 &&
                         dynamic_cast<FileDataItem*>(e.data->second.item));
            return static_cast<FileDataItem*>(e.data->second.item);
        }

    return nullptr;
}

FileDataItem& FileCollectionItem::GetOrAddFile(const char* name)
{
    auto x = GetFileInt(name);
    if (x) return *x;

    auto s = std::find_if(
        secs->entries.begin(), secs->entries.end(),
        [](const auto& e) { return e.name == "FILE_COLLECTION"; });
    BOOST_ASSERT(s != secs->entries.end());
    ++s->count;

    Item* it = this;
    for (; it->GetNext(); it = it->GetNext());
    auto nit = GetContext()->Create<FileDataItem>();
    auto ret = nit.get();
    entries.push_back({FixedString<0x200>{name}, 0, 0,
        GetContext()->CreateLabelFallback(name, {nit.get(), 0})});
    it->InsertAfter(std::move(nit));

    return *ret;
}

void FileCollectionItem::ReplaceFile(
    const char* name, std::unique_ptr<Item> nitem)
{
    auto& dat = GetOrAddFile(name);
    dat.RemoveChildren();
    dat.PrependChild(std::move(nitem));
}

void FileCollectionItem::RedoPadding()
{
    static constexpr size_t PAD_SIZE = 0x40;
    static auto pad_buffer =
        std::make_shared<StringBuffer>(std::string(PAD_SIZE, '\0'));

    Item* it = this;
    while (it) it = PadItem(it);
}

void FileCollectionItem::ExtractTo(const boost::filesystem::path& dir) const
{
    boost::filesystem::create_directories(dir);
    for (const auto& e : entries)
    {
        boost::filesystem::ofstream os;
        os.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        os.open(dir / e.name.c_str(), std::ios_base::out | std::ios_base::binary);
        BOOST_ASSERT(e.data->second.offset == 0);
        e.data->second.item->Dump(os);
    }
}

void FileDataItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);

    os << "file_data {\n" << *GetChildren() << "\n}";
}

}
