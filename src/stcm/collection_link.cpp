#include "collection_link.hpp"
#include "../context.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"

namespace Stcm
{

bool CollectionLinkHeader::IsValid(size_t file_size) const noexcept
{
    return field_00 == 0 &&
        offset <= file_size &&
        offset + sizeof(CollectionLinkEntry)*count <= file_size &&
        field_0c == 0 &&
        field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0 &&
        field_20 == 0 && field_24 == 0 && field_28 == 0 && field_2c == 0 &&
        field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0;
}

bool CollectionLinkEntry::IsValid(size_t file_size) const noexcept
{
    return name_0 <= file_size && name_1 <= file_size && ptr == 0 &&
        field_0c == 0 &&
        field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0;
}

CollectionLinkHeaderItem::CollectionLinkHeaderItem(
    Key k, Context* ctx, const CollectionLinkHeader* s)
    : Item{k, ctx}
{
    if (!s->IsValid(GetContext()->GetSize()))
        throw std::runtime_error("Invalid collection link header");

    data = GetContext()->CreateLabelFallback("collection_link", s->offset);
}

CollectionLinkHeaderItem* CollectionLinkHeaderItem::CreateAndInsert(
    ItemPointer ptr)
{
    auto x = RawItem::Get<CollectionLinkHeader>(ptr);
    size_t count = x.ptr->count;
    if (x.len < sizeof(CollectionLinkHeader))
        throw std::runtime_error("Collection link header: premature end of data");
    auto ret = x.ritem.SplitCreate<CollectionLinkHeaderItem>(ptr.offset, x.ptr);

    auto ptr2 = ret->data->second;
    auto* ritem2 = ptr2.Maybe<RawItem>();
    if (!ritem2)
    {
        // HACK!
        if (ptr2.offset != 0 || count != 0)
            throw std::runtime_error("Collection link: invalid entry pointer");
        auto& eof = ptr2.AsChecked0<EofItem>();
        eof.Replace(eof.GetContext()->Create<CollectionLinkItem>(nullptr, 0));
        return ret;
    }

    auto e = RawItem::Get<CollectionLinkEntry>(ptr2);

    if (e.len < count*sizeof(CollectionLinkItem))
        throw std::runtime_error("Collection link: premature end of data");
    e.ritem.SplitCreate<CollectionLinkItem>(ptr2.offset, e.ptr, count);

    return ret;
}

void CollectionLinkHeaderItem::Dump(std::ostream& os) const
{
    CollectionLinkHeader hdr{};
    hdr.offset = ToFilePos(data->second);
    hdr.count = data->second.As0<CollectionLinkItem>().entries.size();
    os.write(reinterpret_cast<char*>(&hdr), sizeof(CollectionLinkHeader));
}

void CollectionLinkHeaderItem::PrettyPrint(std::ostream& os) const
{
    Item::PrettyPrint(os);
    os << "collection_link_header(@" << data->first << ")";
}

CollectionLinkItem::CollectionLinkItem(
        Key k, Context* ctx, const CollectionLinkEntry* e, size_t count)
    : Item{k, ctx}
{
    entries.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
        if (!e[i].IsValid(GetContext()->GetSize()))
            throw std::runtime_error("Invalid collection link entry");
        entries.push_back({
            GetContext()->GetLabelTo(e[i].name_0),
            GetContext()->GetLabelTo(e[i].name_1)});
    }
}

void CollectionLinkItem::Dump(std::ostream& os) const
{
    CollectionLinkEntry ee{};
    for (const auto& e : entries)
    {
        ee.name_0 = ToFilePos(e.name_0->second);
        ee.name_1 = ToFilePos(e.name_1->second);
        os.write(reinterpret_cast<char*>(&ee), sizeof(CollectionLinkEntry));
    }
}

void CollectionLinkItem::PrettyPrint(std::ostream& os) const
{
    bool good_labels = true;
    for (auto lbl : GetLabels())
        if (lbl.first % sizeof(CollectionLinkEntry) != 0 ||
            lbl.first / sizeof(CollectionLinkEntry) >= entries.size())
        {
            good_labels = false;
            break;
        }
    if (!good_labels)
        Item::PrettyPrint(os);

    size_t i = 0;
    for (const auto& e : entries)
    {
        if (good_labels)
        {
            auto x = GetLabels().equal_range(i);
            for (auto it = x.first; it != x.second; ++it)
                os << '@' << it->second->first << ":\n";
            i += sizeof(CollectionLinkEntry);
        }


        os << "collection_link(@" << e.name_0->first << ", @" << e.name_1->first
           << ")\n";
    }
}


}
