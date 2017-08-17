#ifndef UUID_94EB7F8A_BDE7_47F7_9B17_BF00A9A3EEBB
#define UUID_94EB7F8A_BDE7_47F7_9B17_BF00A9A3EEBB
#pragma once

#include "item.hpp"
#include "../source.hpp"

namespace Neptools
{

class CStringItem final : public Item
{
    LIBSHIT_DYNAMIC_OBJECT;
public:
    CStringItem(Key k, Context& ctx, std::string string)
        : Item{k, ctx}, string{std::move(string)} {}
    CStringItem(Key k, Context& ctx, const Source& src);
    static CStringItem& CreateAndInsert(ItemPointer ptr);
    FilePosition GetSize() const noexcept override { return string.size() + 1; }

    static std::string GetLabelName(std::string string);
    std::string GetLabelName() const { return GetLabelName(string); }

    std::string string;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
};

}

#endif
