#ifndef STCL_GBNL_HPP
#define STCL_GBNL_HPP
#pragma once

#include "../item.hpp"
#include "../gbnl.hpp"

class RawItem;

namespace Stcm
{

class GbnlItem final : public Item, public Gbnl
{
public:
    GbnlItem(Key k, Context* ctx, const Byte* data, size_t len);
    static GbnlItem* CreateAndInsert(RawItem* ritem);

    void Dump(std::ostream& os) const override { Gbnl::Dump(os); }
    void PrettyPrint(std::ostream& os) const override { Gbnl::Inspect(os); }
    size_t GetSize() const noexcept override { return Gbnl::GetSize(); }
};

}

#endif
