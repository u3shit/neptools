#ifndef ITEM_BASE_HPP
#define ITEM_BASE_HPP
#pragma once

#include <cstdint>
#include <functional>
#include "utils.hpp"

class Item;
class Context;

using FilePosition = std::uint32_t;
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

namespace std
{
template<> struct hash<ItemPointer>
{
    std::size_t operator()(const ItemPointer& ptr) const
    {
        return hash<Item*>()(ptr.item) ^ hash<FilePosition>()(ptr.offset);
    }
};
}

#endif
