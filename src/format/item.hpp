#ifndef UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#define UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#pragma once

#include "item_base.hpp"
#include "../dumpable.hpp"
#include "../shared_ptr.hpp"

#include <iosfwd>
#include <vector>
#include <map>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>

namespace Neptools
{

class ItemWithChildren;

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
    RefCountedPtr<ItemWithChildren> GetParent() noexcept
    { return parent.lock(); }

    RefCountedPtr<const Context> GetContext() const noexcept
    { return context.lock(); }
    const Context& GetUnsafeContext() const noexcept
    { return *context.unsafe_get(); }
    RefCountedPtr<const ItemWithChildren> GetParent() const noexcept
    { return parent.lock(); }
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
    WeakRefCountedPtr<ItemWithChildren> parent;

    LabelsContainer labels;

    friend class Context;
    friend class ItemList;
    friend class ItemWithChildren;
};
NEPTOOLS_STATIC_ASSERT(
    std::is_same<SmartPtr<Item>, RefCountedPtr<Item>>::value);

std::ostream& operator<<(std::ostream& os, const Item& item);
inline FilePosition ToFilePos(ItemPointer ptr) noexcept
{ return ptr.item->GetPosition() + ptr.offset; }

class ItemList : private boost::intrusive::list<
    Item, boost::intrusive::constant_time_size<false>>
{
private:
    ItemList() = default;
    ~ItemList() noexcept { clear(); }
    friend class ItemWithChildren;

public:
    // types
    using list::value_traits;
    using list::pointer;
    using list::const_pointer;
    using list::value_type;
    using list::reference;
    using list::const_reference;
    using list::difference_type;
    using list::size_type;
    using list::iterator;
    using list::const_iterator;
    using list::reverse_iterator;
    using list::const_reverse_iterator;
    using list::node_traits;
    using list::node;
    using list::node_ptr;
    using list::const_node_ptr;
    using list::node_algorithms;
    using list::header_holder_type;

    void push_back(reference it) noexcept
    {
        Add(it);
        list::push_back(it);
    }

    void push_front(reference it) noexcept
    {
        Add(it);
        list::push_front(it);
    }

    void pop_back() noexcept { list::pop_back_and_dispose(Disposer{this}); }
    void pop_front() noexcept { list::pop_front_and_dispose(Disposer{this}); }

    using list::front;
    using list::back;
    using list::begin;
    using list::cbegin;
    using list::end;
    using list::cend;
    using list::rbegin;
    using list::crbegin;
    using list::rend;
    using list::crend;
    using list::size;
    using list::empty;
    using list::swap;
    using list::shift_backwards;
    using list::shift_forward;

    iterator erase(const_iterator it) noexcept
    { return list::erase_and_dispose(it, Disposer{this}); }
    iterator erase(const_iterator b, const_iterator e) noexcept
    { return list::erase_and_dispose(b, e, Disposer{this}); }

    void clear() noexcept { list::clear_and_dispose(Disposer{this}); }

    // clone_from?

    iterator insert(const_iterator it, reference val) noexcept
    { Add(val); return list::insert(it, val); }
    template <typename Iterator>
    iterator insert(const_iterator it, Iterator b, Iterator e) noexcept
    {
        for (Iterator i = b; i != e; ++i) Add(*i);
        return list::insert(it, b, e);
    }
    template <typename Iterator>
    void dispose(Iterator b, Iterator e) noexcept
    { clear(); insert(cend(), b, e); }

    //using list::splice; // ref count ok: moved to another list; but parent may change
    using list::sort;
    //using list::merge; // like splice
    using list::reverse;

    // requires operator==
    //void remove(const_reference it)
    //{ list::remove_and_dispose(it, Disposer{this}); }
    template <typename Pred> void remove_if(Pred p)
    { list::remove_and_dispose_if(p, Disposer{this}); }

    // requires operator==
    //void unique() { list::unique_and_dispose(Disposer{}); }
    template <typename Pred> void unique(Pred p)
    { list::unique_and_dispose(p, Disposer{this}); }

    using list::iterator_to;
    using list::check;

    // static funs
    static ItemList& container_from_end_iterator(iterator it) noexcept
    { return static_cast<ItemList&>(list::container_from_end_iterator(it)); }
    static const ItemList& container_from_end_iterator(const_iterator it) noexcept
    { return static_cast<const ItemList&>(list::container_from_end_iterator(it)); }

    using list::s_iterator_to;

    // consts
    using list::constant_time_size;
    using list::stateful_value_traits;
    using list::has_container_from_iterator;

private:
    void Add(reference it);
    struct Disposer
    {
        ItemList* list;
        void operator()(pointer p);
    };
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

    friend class ItemList;
};

}
#endif
