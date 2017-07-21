#ifndef UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#define UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#pragma once

#include "../item.hpp"
#include "../../source.hpp"
#include "../../fixed_string.hpp"
#include "../../lua/auto_table.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stcm
{

class HeaderItem;
class ExportsItem final : public Item
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    enum NEPTOOLS_LUAGEN() Type : uint32_t
    {
        CODE = 0,
        DATA = 1,
    };

    struct Entry
    {
        boost::endian::little_uint32_t type;
        FixedString<0x20> name;
        boost::endian::little_uint32_t offset;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Entry) == 0x28);

    struct EntryType : public RefCounted, public Lua::DynamicObject
    {
        Type type;
        FixedString<0x20> name;
        NotNull<LabelPtr> lbl;

        EntryType(Type type, const FixedString<0x20>& name,
                  NotNull<LabelPtr> lbl)
            : type{type}, name{std::move(name)}, lbl{std::move(lbl)} {}

        NEPTOOLS_DYNAMIC_OBJECT;
    };
    using VectorEntry = NotNull<RefCountedPtr<EntryType>>;

    ExportsItem(Key k, Context& ctx) : Item{k, ctx} {}
    ExportsItem(Key k, Context& ctx, Source src, uint32_t export_count);
    ExportsItem(Key k, Context& ctx, AT<std::vector<VectorEntry>> entries)
        : Item{k, ctx}, entries{std::move(entries.Get())} {}
    static ExportsItem& CreateAndInsert(ItemPointer ptr, uint32_t export_count);

    FilePosition GetSize() const noexcept override
    { return sizeof(Entry) * entries.size(); }

    NEPTOOLS_LUAGEN(get="::Neptools::Lua::GetSmartOwnedMember")
    std::vector<VectorEntry> entries;

    void Dispose() noexcept override;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void Parse_(Context& ctx, Source& src, uint32_t export_count);
};

}
}

NEPTOOLS_ENUM(Neptools::Stcm::ExportsItem::ExportsItem::Type);
#endif
