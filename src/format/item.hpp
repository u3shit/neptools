#ifndef UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#define UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#pragma once

#include "item_base.hpp"
#include "../dumpable.hpp"
#include "../shared_ptr.hpp"
#include "../container/list.hpp"

#include <iosfwd>
#include <vector>
#include <map>
#include <boost/intrusive/set.hpp>

namespace Neptools
{

class ItemWithChildren;
struct ItemListTraits;

class Item : public RefCounted, public Dumpable,
             public boost::intrusive::list_base_hook<LinkMode>
{
protected:
    struct Key {};
public:
    // do not change Context* to Weak/Shared ptr
    // otherwise Context's constructor will try to construct a WeakPtr before
    // RefCounted's constructor is finished, making an off-by-one error and
    // freeing the context twice
    explicit Item(Key, Context* ctx, FilePosition position = 0) noexcept
        : position{position}, context{ctx} {}
    Item(const Item&) = delete;
    void operator=(const Item&) = delete;
    virtual ~Item();

    RefCountedPtr<Context> GetContext() noexcept
    { return context.lock(); }
    Context& GetUnsafeContext() noexcept { return *context.unsafe_get(); }
    ItemWithChildren* GetParent() noexcept { return parent; }

    RefCountedPtr<const Context> GetContext() const noexcept
    { return context.lock(); }
    const Context& GetUnsafeContext() const noexcept
    { return *context.unsafe_get(); }
    const ItemWithChildren* GetParent() const noexcept { return parent; }
    auto Iterator() const noexcept;
    auto Iterator() noexcept;

    FilePosition GetPosition() const noexcept { return position; }

    // requires: has valid parent
    void Replace(NotNull<RefCountedPtr<Item>> nitem) noexcept;

    // properties needed: none (might help if ordered)
    // update Slice if no longer ordered
    using LabelsContainer = boost::intrusive::multiset<
        Label,
        boost::intrusive::base_hook<LabelOffsetHook>,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::key_of_value<LabelOffsetKeyOfValue>>;
    const LabelsContainer& GetLabels() const { return labels; }

    void Dispose() noexcept override;

protected:
    void UpdatePosition(FilePosition npos);

    void Inspect_(std::ostream& os) const override = 0;

    using SlicePair = std::pair<NotNull<RefCountedPtr<Item>>, FilePosition>;
    using SliceSeq = std::vector<SlicePair>;
    void Slice(SliceSeq seq);

    FilePosition position;

private:
    WeakRefCountedPtr<Context> context;
    ItemWithChildren* parent = nullptr;

    LabelsContainer labels;

    friend class Context;
    friend struct ItemListTraits;
    friend class ItemWithChildren;
};
NEPTOOLS_STATIC_ASSERT(
    std::is_same<SmartPtr<Item>, RefCountedPtr<Item>>::value);

std::ostream& operator<<(std::ostream& os, const Item& item);
inline FilePosition ToFilePos(ItemPointer ptr) noexcept
{ return ptr.item->GetPosition() + ptr.offset; }

using ItemList = List<Item, ItemListTraits,
                      boost::intrusive::constant_time_size<false>>;
struct ItemListTraits
{
    static constexpr bool is_movable = false;
    static void add(ItemList& list, Item& item) noexcept;
    static void remove(ItemList& list, Item& item) noexcept;
};
inline auto Item::Iterator() const noexcept
{ return ItemList::s_iterator_to(*this); }
inline auto Item::Iterator() noexcept
{ return ItemList::s_iterator_to(*this); }

class ItemWithChildren : public Item, private ItemList
{
public:
    using Item::Item;

    ItemList& GetChildren() noexcept { return *this; }
    const ItemList& GetChildren() const noexcept { return *this; }

    FilePosition GetSize() const override;
    void Fixup() override { Fixup_(0); }

    void MoveNextToChild(size_t size) noexcept;

    void Dispose() noexcept override;

protected:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& sink) const override;
    void Fixup_(FilePosition offset);

    friend struct ::Neptools::ItemListTraits;
};

}
#endif
