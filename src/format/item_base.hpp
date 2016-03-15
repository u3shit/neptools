#ifndef UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#define UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#pragma once

#include <cstdint>
#include <functional>
#include "../utils.hpp"

namespace Neptools
{

class Item;
class Context;

struct ItemPointer
{
    Item* item;
    FilePosition offset;

    bool operator==(const ItemPointer& o) const
    { return item == o.item && offset == o.offset; }
    bool operator!=(const ItemPointer& o) const
    { return item != o.item || offset != o.offset; }

    template <typename T>
    T& As() const { return *asserted_cast<T*>(item); }

    template <typename T>
    T& AsChecked() const { return dynamic_cast<T&>(*item); }

    template <typename T>
    T* Maybe() const { return dynamic_cast<T*>(item); }

    template <typename T>
    T& As0() const
    {
        BOOST_ASSERT(offset == 0);
        return *asserted_cast<T*>(item);
    }

    template <typename T>
    T& AsChecked0() const
    {
        BOOST_ASSERT(offset == 0);
        return dynamic_cast<T&>(*item);
    }

    template <typename T>
    T* Maybe0() const
    {
        BOOST_ASSERT(offset == 0);
        return dynamic_cast<T*>(item);
    }
};

using Label = std::pair<const std::string, ItemPointer>;

}

namespace std
{
template<> struct hash<::Neptools::ItemPointer>
{
    std::size_t operator()(const ::Neptools::ItemPointer& ptr) const
    {
        return hash<::Neptools::Item*>()(ptr.item) ^
            hash<::Neptools::FilePosition>()(ptr.offset);
    }
};
}

#endif
