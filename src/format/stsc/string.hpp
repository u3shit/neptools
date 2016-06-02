#ifndef UUID_94EB7F8A_BDE7_47F7_9B17_BF00A9A3EEBB
#define UUID_94EB7F8A_BDE7_47F7_9B17_BF00A9A3EEBB
#pragma once

#include "../item.hpp"
#include "../../source.hpp"

namespace Neptools
{
namespace Stsc
{

class StringItem final : public Item
{
public:
    StringItem(Key k, Context* ctx, Source src);
    static StringItem* CreateAndInsert(ItemPointer ptr);
    FilePosition GetSize() const noexcept override { return str.size() + 1; }

    std::string str;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

}
}

#endif
