#ifndef EOF_ITEM_HPP
#define EOF_ITEM_HPP
#pragma once

#include "item.hpp"

class EofItem final : public Item
{
public:
    using Item::Item;

    void Dump(std::ostream&) const override {}
    void PrettyPrint(std::ostream& os) const override { Item::PrettyPrint(os); }
    size_t GetSize() const noexcept override { return 0; }
};

#endif
