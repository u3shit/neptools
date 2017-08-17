#ifndef UUID_2E2EE447_59EF_4611_B8B5_40180CC6FBCC
#define UUID_2E2EE447_59EF_4611_B8B5_40180CC6FBCC
#pragma once

#include "item.hpp"

namespace Neptools
{

class EofItem final : public Item
{
    LIBSHIT_DYNAMIC_OBJECT;
public:
    EofItem(Key k, Context& ctx) : Item{k, ctx} {}

    void Dump_(Sink&) const override {}
    void Inspect_(std::ostream& os, unsigned indent) const override;
    FilePosition GetSize() const noexcept override { return 0; }
};

}
#endif
