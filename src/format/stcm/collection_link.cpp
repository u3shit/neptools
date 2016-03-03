#include "collection_link.hpp"
#include "../context.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"

namespace Stcm
{

void CollectionLinkHeader::Validate(FilePosition file_size) const
{
#define VALIDATE(x) VALIDATE_FIELD("Stcm::CollectionLinkHeader", x)
    VALIDATE(field_00 == 0);
    VALIDATE(offset <= file_size);
    VALIDATE(offset + sizeof(CollectionLinkEntry)*count <= file_size);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
    VALIDATE(field_20 == 0 && field_24 == 0 && field_28 == 0 && field_2c == 0);
    VALIDATE(field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0);
#undef VALIDATE
}

void CollectionLinkEntry::Validate(FilePosition file_size) const
{
#define VALIDATE(x) VALIDATE_FIELD("Stcm::CollectionLinkEntry", x)
    VALIDATE(name_0 <= file_size);
    VALIDATE(name_1 <= file_size);
    VALIDATE(ptr == 0);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
#undef VALIDATE
}

CollectionLinkHeaderItem::CollectionLinkHeaderItem(
    Key k, Context* ctx, const CollectionLinkHeader& s)
    : Item{k, ctx}
{
    s.Validate(GetContext()->GetSize());

    data = GetContext()->CreateLabelFallback("collection_link", s.offset);
}

CollectionLinkHeaderItem* CollectionLinkHeaderItem::CreateAndInsert(
    ItemPointer ptr)
{
    auto x = RawItem::Get<CollectionLinkHeader>(ptr);
    auto ret = x.ritem.SplitCreate<CollectionLinkHeaderItem>(ptr.offset, x.t);

    auto ptr2 = ret->data->second;
    auto* ritem2 = ptr2.Maybe<RawItem>();
    if (!ritem2)
    {
        // HACK!
        VALIDATE_FIELD("Stcm::CollectionLinkHeaderItem",
                       ptr2.offset == 0 && x.t.count == 0);
        auto& eof = ptr2.AsChecked0<EofItem>();
        eof.Replace(eof.GetContext()->Create<CollectionLinkItem>());
        return ret;
    }

    auto e = RawItem::GetSource(ptr2, x.t.count*sizeof(CollectionLinkEntry));

    e.ritem.SplitCreate<CollectionLinkItem>(ptr2.offset, e.src, x.t.count);

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
    Key k, Context* ctx, Source src, uint32_t count)
    : Item{k, ctx}
{
    AddInfo(&CollectionLinkItem::Parse_, ADD_SOURCE(src), this, src, count);
}

void CollectionLinkItem::Parse_(Source& src, uint32_t count)
{
    entries.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto e = src.Read<CollectionLinkEntry>();
        e.Validate(GetContext()->GetSize());
        entries.push_back({
            GetContext()->GetLabelTo(e.name_0),
            GetContext()->GetLabelTo(e.name_1)});
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
