#ifndef UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#define UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#pragma once

#include "../assert.hpp"
#include "../utils.hpp"
#include "../container/intrusive.hpp"
#include "../lua/type_traits.hpp"

#include <cstdint>
#include <functional>
#include <boost/intrusive/set_hook.hpp>

namespace Neptools
{

class Item;
class Context;

struct ItemPointer
{
    Item* item;
    FilePosition offset;

    ItemPointer(Item* item, FilePosition offset = 0)
        : item{item}, offset{offset} {}
    ItemPointer(Item& item, FilePosition offset = 0)
        : item{&item}, offset{offset} {}

    bool operator==(const ItemPointer& o) const
    { return item == o.item && offset == o.offset; }
    bool operator!=(const ItemPointer& o) const
    { return item != o.item || offset != o.offset; }

    Item* operator->() const { return item; }

    template <typename T>
    T& As() const { return *asserted_cast<T*>(item); }

    template <typename T>
    T& AsChecked() const { return dynamic_cast<T&>(*item); }

    template <typename T>
    T* Maybe() const { return dynamic_cast<T*>(item); }

    template <typename T>
    T& As0() const
    {
        NEPTOOLS_ASSERT(offset == 0);
        return *asserted_cast<T*>(item);
    }

    template <typename T>
    T& AsChecked0() const
    {
        NEPTOOLS_ASSERT(offset == 0);
        return dynamic_cast<T&>(*item);
    }

    template <typename T>
    T* Maybe0() const
    {
        NEPTOOLS_ASSERT(offset == 0);
        return dynamic_cast<T*>(item);
    }
};

template<> struct Lua::TupleLike<ItemPointer>
{
    template <size_t I> static auto& Get(const ItemPointer& ptr) noexcept
    {
        if constexpr (I == 0) return *ptr.item;
        else if constexpr (I == 1) return ptr.offset;
    }
    static constexpr size_t SIZE = 2;
};

using LabelNameHook = boost::intrusive::set_base_hook<
    boost::intrusive::tag<struct NameTag>,
    boost::intrusive::optimize_size<true>, LinkMode>;
using LabelOffsetHook = boost::intrusive::set_base_hook<
    boost::intrusive::tag<struct OffsetTag>,
    boost::intrusive::optimize_size<true>, LinkMode>;

struct Label : LabelNameHook, LabelOffsetHook
{
    std::string name;
    ItemPointer ptr;

    Label(std::string name, ItemPointer ptr)
        : name{std::move(name)}, ptr{ptr} {}
    // prevent accidental copying with auto x = ...;
    Label(const Label&) = delete;
    void operator=(const Label&) = delete;
};

// push as native table
template<> struct Lua::TypeTraits<Label>
{
    static void Push(StateRef vm, const Label& l);
};

// to be used by boost::intrusive::set
struct LabelKeyOfValue
{
    using type = std::string;
    const type& operator()(const Label& l) { return l.name; }
};

struct LabelOffsetKeyOfValue
{
    using type = FilePosition;
    const type& operator()(const Label& l) { return l.ptr.offset; }
};

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
