#ifndef UUID_4CADE91E_2AF1_47AF_8425_9AA799509BFD
#define UUID_4CADE91E_2AF1_47AF_8425_9AA799509BFD
#pragma once

#include <vector>
#include <boost/endian/arithmetic.hpp>
#include <boost/filesystem/path.hpp>

#include "../source.hpp"
#include "../sink.hpp"
#include "../fixed_string.hpp"
#include "../container/ordered_map.hpp"

namespace Neptools
{

namespace Stcm { class File; }

class Cl3 : public Dumpable
{
public:
    struct Header
    {
        char magic[4];
        boost::endian::little_uint32_t field_04;
        boost::endian::little_uint32_t field_08;
        boost::endian::little_uint32_t sections_count;
        boost::endian::little_uint32_t sections_offset;
        boost::endian::little_uint32_t field_14;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Header) == 0x18);

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

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Section) == 0x50);

    struct FileEntry
    {
        FixedString<0x200> name;
        boost::endian::little_uint32_t field_200;
        boost::endian::little_uint32_t data_offset;
        boost::endian::little_uint32_t data_size;
        boost::endian::little_uint32_t link_start;
        boost::endian::little_uint32_t link_count;
        boost::endian::little_uint32_t field_214;
        boost::endian::little_uint32_t field_218;
        boost::endian::little_uint32_t field_21c;
        boost::endian::little_uint32_t field_220;
        boost::endian::little_uint32_t field_224;
        boost::endian::little_uint32_t field_228;
        boost::endian::little_uint32_t field_22c;

        void Validate(uint32_t block_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(FileEntry) == 0x230);

    struct LinkEntry
    {
        boost::endian::little_uint32_t field_00;
        boost::endian::little_uint32_t linked_file_id;
        boost::endian::little_uint32_t link_id;
        boost::endian::little_uint32_t field_0c;
        boost::endian::little_uint32_t field_10;
        boost::endian::little_uint32_t field_14;
        boost::endian::little_uint32_t field_18;
        boost::endian::little_uint32_t field_1c;

        void Validate(uint32_t i, uint32_t file_count) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(LinkEntry) == 0x20);

    Cl3() : field_14{0} {}
    Cl3(Source src);

    void Fixup() override;
    FilePosition GetSize() const override;

    uint32_t field_14;

    struct Entry : public OrderedMapItem
    {
        std::string name;
        uint32_t field_200 = 0;
        std::vector<WeakRefCountedPtr<Entry>> links;

        SmartPtr<Dumpable> src;

        explicit Entry(std::string name) : name{std::move(name)} {}
        Entry(std::string name, uint32_t field_200, SmartPtr<Dumpable> src)
            : name{std::move(name)}, field_200{field_200}, src{std::move(src)} {}
    };
    struct EntryKeyOfValue
    {
        using type = std::string;
        const type& operator()(const Entry& e) { return e.name; }
    };
    OrderedMap<Entry, struct EntryKeyOfValue> entries;
    uint32_t IndexOf(const WeakPtr<Entry>& ptr) const noexcept;

    Entry& GetOrCreateFile(StringView fname);

    void ExtractTo(const boost::filesystem::path& dir) const;
    void UpdateFromDir(const boost::filesystem::path& dir);

    Stcm::File& GetStcm();

private:
    FilePosition data_size;
    unsigned link_count;

    void Parse_(Source& src);
    void Dump_(Sink& os) const override;
    void Inspect_(std::ostream& os) const override;
};

}
#endif
