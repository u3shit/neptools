#ifndef STCM_EXPORTS_HPP
#define STCM_EXPORTS_HPP
#pragma once

#include "../item.hpp"
#include "../fixed_string.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

struct ExportEntry
{
    boost::endian::little_uint32_t field_0;
    FixedString<0x20> name;
    boost::endian::little_uint32_t offset;

    bool IsValid(size_t file_size) const noexcept;
};
static_assert(sizeof(ExportEntry) == 0x28, "");

class HeaderItem;
class Exports : public Item
{
public:
    Exports(Key k, Context* ctx, const HeaderItem* hdr);

    void Dump(std::ostream& os) const;
    size_t GetSize() const noexcept { return sizeof(ExportEntry) * entries.size(); }

private:
    using EntryType = std::pair<FixedString<0x20>, const Label*>;
    std::vector<EntryType> entries;
};

}

#endif
