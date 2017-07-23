#ifndef UUID_CB40AD6D_5157_4C96_A9A9_371A9F215955
#define UUID_CB40AD6D_5157_4C96_A9A9_371A9F215955
#pragma once

#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stcm
{

class DataItem final : public ItemWithChildren
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    struct Header
    {
        boost::endian::little_uint32_t type;
        boost::endian::little_uint32_t offset_unit;
        boost::endian::little_uint32_t field_8;
        boost::endian::little_uint32_t length;

        void Validate(FilePosition chunk_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Header) == 0x10);

    DataItem(Key k, Context& ctx, uint32_t type, uint32_t offset_unit,
             uint32_t field_8)
        : ItemWithChildren{k, ctx}, type{type}, offset_unit{offset_unit},
          field_8{field_8} {}
    NEPTOOLS_NOLUA
    DataItem(Key k, Context& ctx, const Header& hdr, size_t chunk_size);
    static DataItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override;
    void Fixup() override;

    uint32_t type, offset_unit, field_8;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
};

class DataFactory
{
public:
    using Fun = bool (*)(DataItem& it);
    DataFactory(Fun f) { GetStore().push_back(f); }

    static void Check(DataItem& it);

private:
    using Store = std::vector<Fun>;
    static Store& GetStore()
    {
        static Store store;
        return store;
    }
};

}
}
#endif
