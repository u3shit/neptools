#ifndef UUID_51AA015D_E824_48D3_8BB4_37BC559302DA
#define UUID_51AA015D_E824_48D3_8BB4_37BC559302DA
#pragma once

#include "../item.hpp"
#include "../../source.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

struct CollectionLinkHeader
{
    boost::endian::little_uint32_t field_00;
    boost::endian::little_uint32_t offset;
    boost::endian::little_uint32_t count;
    boost::endian::little_uint32_t field_0c;
    boost::endian::little_uint32_t field_10;
    boost::endian::little_uint32_t field_14;
    boost::endian::little_uint32_t field_18;
    boost::endian::little_uint32_t field_1c;
    boost::endian::little_uint32_t field_20;
    boost::endian::little_uint32_t field_24;
    boost::endian::little_uint32_t field_28;
    boost::endian::little_uint32_t field_2c;
    boost::endian::little_uint32_t field_30;
    boost::endian::little_uint32_t field_34;
    boost::endian::little_uint32_t field_38;
    boost::endian::little_uint32_t field_3c;

    void Validate(FilePosition file_size) const;
};
STATIC_ASSERT(sizeof(CollectionLinkHeader) == 0x40);

struct CollectionLinkEntry
{
    boost::endian::little_uint32_t name_0;
    boost::endian::little_uint32_t name_1;
    boost::endian::little_uint32_t ptr; // filled by engine
    boost::endian::little_uint32_t field_0c;
    boost::endian::little_uint32_t field_10;
    boost::endian::little_uint32_t field_14;
    boost::endian::little_uint32_t field_18;
    boost::endian::little_uint32_t field_1c;

    void Validate(FilePosition file_size) const;
};
STATIC_ASSERT(sizeof(CollectionLinkEntry) == 0x20);

class CollectionLinkHeaderItem final : public Item
{
public:
    CollectionLinkHeaderItem(Key k, Context* ctx, const CollectionLinkHeader& s);
    static CollectionLinkHeaderItem* CreateAndInsert(ItemPointer ptr);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    FilePosition GetSize() const noexcept override
    { return sizeof(CollectionLinkHeader); }

    const Label* data;
};

class CollectionLinkItem final : public Item
{
public:
    CollectionLinkItem(Key k, Context* ctx) : Item{k, ctx} {}
    CollectionLinkItem(Key k, Context* ctx, Source src, uint32_t count);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    FilePosition GetSize() const noexcept override
    { return entries.size() * sizeof(CollectionLinkEntry); }

    struct Entry
    {
        const Label* name_0;
        const Label* name_1;
    };
    std::vector<Entry> entries;

private:
    void Parse_(Source& src, uint32_t count);
};

}

#endif
