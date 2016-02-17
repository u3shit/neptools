#ifndef UUID_D5A0B803_8837_41BA_8C9E_369C6FCE0E7F
#define UUID_D5A0B803_8837_41BA_8C9E_369C6FCE0E7F
#pragma once

#include "../../fixed_string.hpp"
#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Cl3
{

struct Section
{
    FixedString<0x20> name;
    boost::endian::little_uint32_t count;
    boost::endian::little_uint32_t data_size;
    boost::endian::little_uint32_t data_offset;
    boost::endian::little_uint32_t field_2c;
    boost::endian::little_uint32_t field_30;
    boost::endian::little_uint32_t field_34;
    boost::endian::little_uint32_t field_38;
    boost::endian::little_uint32_t field_3c;
    boost::endian::little_uint32_t field_40;
    boost::endian::little_uint32_t field_44;
    boost::endian::little_uint32_t field_48;
    boost::endian::little_uint32_t field_4c;

    bool IsValid(size_t file_size) const noexcept;
};
STATIC_ASSERT(sizeof(Section) == 0x50);

class HeaderItem;
class SectionEntryItem;
class SectionsItem final : public Item
{
public:
    SectionsItem(Key k, Context* ctx) : Item{k, ctx} {}
    SectionsItem(Key k, Context* ctx, const Section* s, size_t count);
    static SectionsItem* CreateAndInsert(ItemPointer ptr, size_t num_sections);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override
    { return entries.size() * sizeof(Section); }

    const SectionEntryItem* GetEntry(const char* name) const noexcept
    { return GetEntryInt(name); }
    const SectionEntryItem* GetEntry(const std::string& name) const noexcept
    { return GetEntryInt(name.c_str()); }
    SectionEntryItem* GetEntry(const char* name) noexcept
    { return GetEntryInt(name); }
    SectionEntryItem* GetEntry(const std::string& name) noexcept
    { return GetEntryInt(name.c_str()); }

    struct Entry
    {
        FixedString<0x20> name;
        uint32_t count;
        const Label* data;
    };

    std::vector<Entry> entries;

private:
    SectionEntryItem* GetEntryInt(const char* name) const noexcept;
};

class SectionEntryItem final : public ItemWithChildren
{
public:
    using ItemWithChildren::ItemWithChildren;

    void PrettyPrint(std::ostream& os) const override;
};

}

#endif
