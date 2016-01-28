#ifndef ITEM_BASE_HPP
#define ITEM_BASE_HPP
#pragma once

#include <cstdint>
#include <functional>
#include <boost/operators.hpp>

class Item;
class Context;

using FilePointer = std::uint32_t;
struct ItemPointer : boost::equality_comparable<ItemPointer>
{
    Item* item;
    FilePointer offset;

    bool operator==(const ItemPointer& o) const
    { return item == o.item && offset == o.offset; }
};

using Label = std::pair<const std::string, ItemPointer>;

namespace std
{
template<> struct hash<ItemPointer>
{
    std::size_t operator()(const ItemPointer& ptr) const
    {
        return hash<Item*>()(ptr.item) ^ hash<FilePointer>()(ptr.offset);
    }
};
}

#endif
