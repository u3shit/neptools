#ifndef UUID_FC9891E9_0F56_4675_A342_B057BE8A0A88
#define UUID_FC9891E9_0F56_4675_A342_B057BE8A0A88
#pragma once

#include "item.hpp"

template <typename Ctx>
class ContextItem : public Ctx, public Item
{
public:
    template <typename... Args>
    ContextItem(Key k, Context* ctx, Args&&... args)
        : Ctx{std::forward<Args>(args)...}, Item{k, ctx} {}

    void Dump(std::ostream& os) const override { Ctx::Dump(os); }
    void PrettyPrint(std::ostream& os) const override
    { os << *static_cast<const Ctx*>(this); }
    size_t GetSize() const noexcept override { return Ctx::GetSize(); }

    size_t UpdatePositions(FilePosition npos) override
    { /*Ctx::Fixup();*/ return Item::UpdatePositions(npos); }
};

#endif
