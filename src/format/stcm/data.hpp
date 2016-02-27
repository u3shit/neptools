#ifndef UUID_CB40AD6D_5157_4C96_A9A9_371A9F215955
#define UUID_CB40AD6D_5157_4C96_A9A9_371A9F215955
#pragma once

#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

struct DataHeader
{
    boost::endian::little_uint32_t type;
    boost::endian::little_uint32_t offset_unit;
    boost::endian::little_uint32_t field_8;
    boost::endian::little_uint32_t length;

    bool IsValid(size_t chunk_size) const noexcept;
};
STATIC_ASSERT(sizeof(DataHeader) == 0x10);

class DataItem final : public Item
{
public:
    DataItem(Key k, Context* ctx, const DataHeader& hdr, size_t chunk_size);
    static DataItem* CreateAndInsert(ItemPointer ptr);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override;
    size_t UpdatePositions(FilePosition npos) override;

    uint32_t type, offset_unit, field_8;
};

}

#endif
