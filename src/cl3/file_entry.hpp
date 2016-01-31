#ifndef CL3_FILE_ENTRY_HPP
#define CL3_FILE_ENTRY_HPP
#pragma once

#include "../fixed_string.hpp"
#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Cl3
{

struct FileEntry
{
    FixedString<0x200> name;
    boost::endian::little_uint32_t field_200;
    boost::endian::little_uint32_t data_offset;
    boost::endian::little_uint32_t data_size;
    boost::endian::little_uint32_t field_20c;
    boost::endian::little_uint32_t field_210;
    boost::endian::little_uint32_t field_214;
    boost::endian::little_uint32_t field_218;
    boost::endian::little_uint32_t field_21c;
    boost::endian::little_uint32_t field_220;
    boost::endian::little_uint32_t field_224;
    boost::endian::little_uint32_t field_228;
    boost::endian::little_uint32_t field_22c;

    bool IsValid(size_t file_size) const noexcept;
};
static_assert(sizeof(FileEntry) == 0x230, "");

class FileEntriesItem final : public Item
{
public:
    FileEntriesItem(Key k, Context* ctx, const FileEntry* e, size_t count);
    static FileEntriesItem* CreateAndInsert(RawItem* ritem, size_t count);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override
    { return entries.size() * sizeof(FileEntry); }

    struct Entry
    {
        FixedString<0x200> name;
        uint32_t field_20c, field_210;
        const Label* data;
    };

    std::vector<Entry> entries;
};

class FileDataItem final : public ItemWithChildren
{
public:
    using ItemWithChildren::ItemWithChildren;

    void PrettyPrint(std::ostream& os) const override;
};

}

#endif
