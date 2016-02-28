#ifndef UUID_2E2EE447_59EF_4611_B8B5_40180CC6FBCC
#define UUID_2E2EE447_59EF_4611_B8B5_40180CC6FBCC
#pragma once

#include "item.hpp"

class EofItem final : public Item
{
public:
    using Item::Item;

    void Dump(std::ostream&) const override {}
    void PrettyPrint(std::ostream& os) const override { Item::PrettyPrint(os); }
    FilePosition GetSize() const noexcept override { return 0; }
};

#endif
