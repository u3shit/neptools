#include "file.hpp"
#include "file_collection.hpp"
#include "header.hpp"
#include "sections.hpp"
#include "../item.hpp"

#include <boost/assert.hpp>

namespace Cl3
{

File::File()
{
    auto hdr = Create<HeaderItem>();
    auto sec = Create<SectionsItem>();
    auto sec_entry = Create<SectionEntryItem>();
    auto file_coll = Create<FileCollectionItem>(sec.get());

    sec_entry->PrependChild(std::move(file_coll));
    sec->entries.push_back({
        FixedString<0x20>{"FILE_COLLECTION"},
        0,
        CreateLabelFallback("FILE_COLLECTION", {sec_entry.get(), 0})
    });

    hdr->num_sections = 1;
    hdr->sections = CreateLabelFallback("sections", {sec.get(), 0});

    Item* ptr = hdr.get();
    PadItem(ptr);
    ptr = ptr->GetNext();

    ptr->InsertAfter(std::move(sec));
    ptr = ptr->GetNext();
    PadItem(ptr);
    ptr = ptr->GetNext();

    ptr->InsertAfter(std::move(sec_entry));

    SetRoot(std::move(hdr));
}

File::File(std::shared_ptr<Buffer> buf, size_t offset, size_t len)
{
    auto root = Create<RawItem>(std::move(buf), offset, len);
    auto root_sav = root.get();
    SetRoot(std::move(root));
    HeaderItem::CreateAndInsert({root_sav, 0});
}

void File::Fixup()
{
    auto fc = GetHeader().GetSections().GetEntry("FILE_COLLECTION");
    if (fc)
        asserted_cast<FileCollectionItem*>(fc->GetChildren())->RedoPadding();

    Context::Fixup();
}

const HeaderItem& File::GetHeader() const noexcept
{
    return *asserted_cast<const HeaderItem*>(GetRoot());
}

HeaderItem& File::GetHeader() noexcept
{
    return *asserted_cast<HeaderItem*>(GetRoot());
}

template <typename Ret, typename Ref>
static Ret& GetFileCollection2(Ref& file)
{
    auto x = file.GetHeader().GetSections().GetEntry("FILE_COLLECTION");
    if (!x)
        throw std::runtime_error("Invalid CL3 file: no FILE_COLLECTION");

    return *asserted_cast<Ret*>(x->GetChildren());
}

const FileCollectionItem& File::GetFileCollection() const
{ return GetFileCollection2<const FileCollectionItem>(*this); }

FileCollectionItem& File::GetFileCollection()
{ return GetFileCollection2<FileCollectionItem>(*this); }

static void CheckAndRemove(RawItem& ritem)
{
    for (size_t i = 0; i < ritem.GetSize(); ++i)
        if (ritem[i] != 0)
            throw std::runtime_error("Invalid padding inside CL3");
    ritem.Remove();
}

Item* PadItem(Item* it)
{
    static constexpr size_t PAD_SIZE = 0x40;
    static auto pad_buffer =
        std::make_shared<StringBuffer>(std::string(PAD_SIZE, '\0'));

    auto next = dynamic_cast<RawItem*>(it->GetNext());
    auto pad = next ? next->GetSize() : 0;
    auto required_pad = (PAD_SIZE - (it->GetSize() % PAD_SIZE)) % PAD_SIZE;
    if (pad != required_pad)
    {
        if (next)
        {
            CheckAndRemove(*next);
            next = nullptr;
        }
        if (required_pad != 0)
        {
            auto n = it->GetContext()->Create<RawItem>(
                pad_buffer, 0, required_pad);
            next = n.get();
            it->InsertAfter(std::move(n));
        }
    }
    return (next ? next : it)->GetNext();
}

}
