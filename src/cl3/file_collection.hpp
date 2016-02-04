#ifndef CL3_FILE_COLLECTION_HPP
#define CL3_FILE_COLLECTION_HPP
#pragma once

#include "../fixed_string.hpp"
#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>

namespace Cl3
{

struct FileEntry
{
    FixedString<0x200> name;
    boost::endian::little_uint32_t field_200;
    boost::endian::little_uint32_t data_offset;
    boost::endian::little_uint32_t data_size;
    boost::endian::little_uint32_t field_20c; // offset
    boost::endian::little_uint32_t field_210; // count
    boost::endian::little_uint32_t field_214;
    boost::endian::little_uint32_t field_218;
    boost::endian::little_uint32_t field_21c;
    boost::endian::little_uint32_t field_220;
    boost::endian::little_uint32_t field_224;
    boost::endian::little_uint32_t field_228;
    boost::endian::little_uint32_t field_22c;

    bool IsValid(size_t rem_size) const noexcept;
};
static_assert(sizeof(FileEntry) == 0x230, "");

class FileDataItem;
class SectionsItem;
class FileCollectionItem final : public Item
{
public:
    FileCollectionItem(Key k, Context* ctx, SectionsItem* secs)
        : Item{k, ctx}, secs{secs} {}
    FileCollectionItem(Key k, Context* ctx, FilePosition pos,
                       const FileEntry* e, size_t count, SectionsItem* secs);
    static FileCollectionItem* CreateAndInsert(
        RawItem* ritem, size_t count, SectionsItem* secs);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override
    { return entries.size() * sizeof(FileEntry); }

    FileDataItem* GetFile(const char* name) noexcept
    { return GetFileInt(name); }
    FileDataItem* GetFile(const std::string& name) noexcept
    { return GetFileInt(name.c_str()); }
    const FileDataItem* GetFile(const char* name) const noexcept
    { return GetFileInt(name); }
    const FileDataItem* GetFile(const std::string& name) const noexcept
    { return GetFileInt(name.c_str()); }

    FileDataItem& GetOrAddFile(const char* name);
    FileDataItem& GetOrAddFile(const std::string& name)
    { return GetOrAddFile(name.c_str()); }

    void ReplaceFile(const char* name, std::unique_ptr<Item> nitem);
    void ReplaceFile(const std::string& name, std::unique_ptr<Item> nitem)
    { ReplaceFile(name.c_str(), std::move(nitem)); }
    void RedoPadding();

    void ExtractTo(const boost::filesystem::path& dir) const;

    struct Entry
    {
        FixedString<0x200> name;
        uint32_t field_20c, field_210;
        const Label* data;
    };

    std::vector<Entry> entries;
    SectionsItem* secs;

private:
    FileDataItem* GetFileInt(const char* name) const noexcept;
};

class FileDataItem final : public ItemWithChildren
{
public:
    using ItemWithChildren::ItemWithChildren;

    void PrettyPrint(std::ostream& os) const override;
};

}

#endif
