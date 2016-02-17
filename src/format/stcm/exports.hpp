#ifndef UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#define UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#pragma once

#include "../item.hpp"
#include "../../fixed_string.hpp"
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
    static ExportsItem* CreateAndInsert(ItemPointer ptr, size_t export_count);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override
    { return sizeof(ExportEntry) * entries.size(); }

    using EntryType = std::pair<FixedString<0x20>, const Label*>;
    std::vector<EntryType> entries;
};

}

#endif
