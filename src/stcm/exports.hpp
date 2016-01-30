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
class ExportsItem final : public Item
{
public:
    ExportsItem(Key k, Context* ctx, const ExportEntry* e, size_t export_count);
    static ExportsItem* CreateAndInsert(const HeaderItem* hdr);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override
    { return sizeof(ExportEntry) * entries.size(); }

    using EntryType = std::pair<FixedString<0x20>, const Label*>;
    std::vector<EntryType> entries;
};

}

#endif
