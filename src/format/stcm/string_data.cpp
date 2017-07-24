#include "string_data.hpp"

#include "data.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

namespace Neptools::Stcm
{

RefCountedPtr<StringDataItem> StringDataItem::MaybeCreateAndReplace(DataItem& it)
{
    // string: data(0, x, 1), where x = size/4, size is strlen+1 rounded to 4 bytes
    // ignore x == 1: it's probably an int32
    if (it.type != 0 || it.offset_unit <= 1 || it.field_8 != 1 ||
        it.GetChildren().empty() || // only one child
        &it.GetChildren().front() != &it.GetChildren().back()) return nullptr;
    auto child = dynamic_cast<RawItem*>(&it.GetChildren().front());
    if (!child || child->GetSize() != it.offset_unit * 4) return nullptr;

    auto src = child->GetSource();
    auto s = src.ReadCString();
    auto padlen = it.offset_unit * 4 - s.size() - 1;
    if (padlen > 4) return nullptr;
    char pad[4];
    src.Read(pad, padlen);
    // check padding all zero. I don't think it's required, but in the game
    // files they're zero filled, + dump will generate zeros, so do not lose
    // information by discarding a non-null padding...
    for (size_t i = 0; i < padlen; ++i)
        if (pad[i] != 0) return nullptr;

    auto sit = it.GetContext()->Create<StringDataItem>(std::move(s));
    it.Replace(sit);
    return sit;
}

FilePosition StringDataItem::GetSize() const noexcept
{
    return sizeof(DataItem::Header) + (string.length() + 1 + 3) / 4 * 4;
}

void StringDataItem::Dump_(Sink& sink) const
{
    auto len = (string.length() + 1 + 3) / 4 * 4;
    sink.WriteGen(DataItem::Header{0, len/4, 1, len});
    sink.Write(string);
    sink.Pad(len - string.length());
}

void StringDataItem::Inspect_(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);
    os << "string_data(" << Quoted(string) << ')';
}

static Stcm::DataFactory reg{[](DataItem& it) {
    return !!StringDataItem::MaybeCreateAndReplace(it); }};

}

#include "string_data.binding.hpp"
