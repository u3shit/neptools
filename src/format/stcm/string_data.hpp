#ifndef UUID_5AE4EF4E_E8EB_4A3D_90A7_58FA73AF7B71
#define UUID_5AE4EF4E_E8EB_4A3D_90A7_58FA73AF7B71
#pragma once

#include "../item.hpp"

namespace Neptools::Stcm
{
class DataItem;

class StringDataItem final : public Item
{
    LIBSHIT_DYNAMIC_OBJECT;
public:
    StringDataItem(Key k, Context& ctx, std::string str)
        : Item{k, ctx}, string{std::move(str)} {}

    static Libshit::RefCountedPtr<StringDataItem>
    MaybeCreateAndReplace(DataItem& it);

    FilePosition GetSize() const noexcept override;

    std::string string;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
};

}

#endif
