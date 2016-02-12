#include "dynamic_struct.hpp"

namespace DynamicStruct
{

boost::intrusive_ptr<const StructType> StructTypeBuilder::Build()
{
    auto ptr = operator new(
        sizeof(StructType) + (desc.size() - 1) * sizeof(StructType::Item));
    boost::intrusive_ptr<StructType> ret{static_cast<StructType*>(ptr), false};
    ret->refcount = 1;
    ret->item_count = desc.size();

    size_t offs = 0;
    for (size_t i = 0; i < desc.size(); ++i)
    {
        ret->items[i].desc = desc[i];
        ret->items[i].offset = offs;

        offs += desc[i]->size;
        auto al = desc[i]->align;
        offs = (offs + al - 1) / al * al;
    }
    ret->size = offs;

    return ret;
}

Struct::Struct(const boost::intrusive_ptr<const StructType>& type)
    : type{type}
{
    data.reset(new char[type->size]);
    for (size_t i = 0; i < type->item_count; ++i)
        type->items[i].desc->default_construct(&data[type->items[i].offset]);
}

Struct& Struct::operator=(Struct&& o)
{
    if (this != &o)
    {
        type = o.type;
        data = std::move(o.data);
        o.type = nullptr;
        o.data = nullptr;
    }
    return *this;
}

Struct::~Struct()
{
    for (size_t i = 0; i < type->item_count; ++i)
        type->items[i].desc->destruct(&data[type->items[i].offset]);
}

}
