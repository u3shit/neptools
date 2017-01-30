#ifndef UUID_4CADE91E_2AF1_47AF_8425_9AA799509BFD
#define UUID_4CADE91E_2AF1_47AF_8425_9AA799509BFD
#pragma once

#include "../endian.hpp"
#include "../fixed_string.hpp"
#include "../source.hpp"
#include "../sink.hpp"
#include "../container/ordered_map.hpp"

#include <vector>
#include <boost/filesystem/path.hpp>

namespace Neptools
{

namespace Stcm { class File; }

class Cl3 : public Dumpable
{
public:
    template <Order O>
    struct Header
    {
        char magic[3];
        char endian;
        EndianUint32<O> field_04;
        EndianUint32<O> field_08;
        EndianUint32<O> sections_count;
        EndianUint32<O> sections_offset;
        EndianUint32<O> field_14;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Header<Order::little>) == 0x18);
    NEPTOOLS_STATIC_ASSERT(sizeof(Header<Order::big>) == 0x18);

    template <Order O>
    struct Section
    {
        FixedString<0x20> name;
        EndianUint32<O> count;
        EndianUint32<O> data_size;
        EndianUint32<O> data_offset;
        EndianUint32<O> field_2c;
        EndianUint32<O> field_30;
        EndianUint32<O> field_34;
        EndianUint32<O> field_38;
        EndianUint32<O> field_3c;
        EndianUint32<O> field_40;
        EndianUint32<O> field_44;
        EndianUint32<O> field_48;
        EndianUint32<O> field_4c;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Section<Order::little>) == 0x50);

    template <Order O>
    struct FileEntry
    {
        FixedString<0x200> name;
        EndianUint32<O> field_200;
        EndianUint32<O> data_offset;
        EndianUint32<O> data_size;
        EndianUint32<O> link_start;
        EndianUint32<O> link_count;
        EndianUint32<O> field_214;
        EndianUint32<O> field_218;
        EndianUint32<O> field_21c;
        EndianUint32<O> field_220;
        EndianUint32<O> field_224;
        EndianUint32<O> field_228;
        EndianUint32<O> field_22c;

        void Validate(uint32_t block_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(FileEntry<Order::little>) == 0x230);

    template <Order O>
    struct LinkEntry
    {
        EndianUint32<O> field_00;
        EndianUint32<O> linked_file_id;
        EndianUint32<O> link_id;
        EndianUint32<O> field_0c;
        EndianUint32<O> field_10;
        EndianUint32<O> field_14;
        EndianUint32<O> field_18;
        EndianUint32<O> field_1c;

        void Validate(uint32_t i, uint32_t file_count) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(LinkEntry<Order::little>) == 0x20);

    Cl3() : endian{Order::little}, field_14{0} {}
    Cl3(Source src);

    void Fixup() override;
    FilePosition GetSize() const override;

    Order endian;
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
    uint32_t IndexOf(const WeakSmartPtr<Entry>& ptr) const noexcept;

    Entry& GetOrCreateFile(StringView fname);

    void ExtractTo(const boost::filesystem::path& dir) const;
    void UpdateFromDir(const boost::filesystem::path& dir);

    Stcm::File& GetStcm();

private:
    FilePosition data_size;
    unsigned link_count;

    void Parse_(Source& src);
    template <Order O> void DoParse(Source& src, const Header<O>& hdr);

    void Dump_(Sink& os) const override;
    template <Order O> void DoDump(Sink& os) const;
    void Inspect_(std::ostream& os) const override;
};

}
#endif
