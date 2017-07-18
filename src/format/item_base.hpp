#ifndef UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#define UUID_02043882_EC07_4CCA_BD13_1BB9F5C7DB9F
#pragma once

#include "../assert.hpp"
#include "../meta.hpp"
#include "../shared_ptr.hpp"
#include "../utils.hpp"
#include "../container/intrusive.hpp"
#include "../lua/dynamic_object.hpp"
#include "../lua/function_call_types.hpp"
#include "../lua/type_traits.hpp"

#include <cstdint>
#include <functional>
#include <boost/intrusive/set_hook.hpp>

namespace Neptools NEPTOOLS_META(alias_file src/format/item.hpp)
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

    Item& operator*() const { return *item; }
    Item* operator->() const { return &*item; }

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

class Label final : public RefCounted, public Lua::DynamicObject,
                    public LabelNameHook, public LabelOffsetHook
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:

    Label(std::string name, ItemPointer ptr)
        : name{std::move(name)}, ptr{ptr} {}

    const std::string& GetName() const { return name; }
    const ItemPointer& GetPtr() const { return ptr; }

    friend class Context;
    friend class Item;
private:
    std::string name;
    ItemPointer ptr;
};

using LabelPtr = RefCountedPtr<Label>;
using WeakLabelPtr = WeakRefCountedPtr<Label>;

// to be used by boost::intrusive::set
struct LabelKeyOfValue
{
    using type = std::string;
    const type& operator()(const Label& l) { return l.GetName(); }
};

struct LabelOffsetKeyOfValue
{
    using type = FilePosition;
    const type& operator()(const Label& l) { return l.GetPtr().offset; }
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
