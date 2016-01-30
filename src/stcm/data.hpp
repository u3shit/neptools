#ifndef STCM_DATA_HPP
#define STCM_DATA_HPP
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
static_assert(sizeof(DataHeader) == 0x10, "");

class DataItem final : public Item
{
public:
    DataItem(Key k, Context* ctx, const DataHeader* hdr, size_t chunk_size);
    static void MaybeCreate(ItemPointer ptr);
    static DataItem* CreateAndInsert(ItemPointer ptr);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override;
    void UpdatePositions(FilePosition npos) override;

    uint32_t type, offset_unit;
};

}

#endif
