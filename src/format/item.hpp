#ifndef UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#define UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#pragma once

#include "item_base.hpp"
#include "../dumpable.hpp"
#include "../container/parent_list.hpp"

#include <libshit/check.hpp>
#include <libshit/shared_ptr.hpp>
#include <libshit/lua/user_type_fwd.hpp>

#include <iosfwd>
#include <vector>
#include <map>
#include <boost/intrusive/set.hpp>

namespace Neptools
{

class ItemWithChildren;
struct ItemListTraits;

LIBSHIT_GEN_EXCEPTION_TYPE(InvalidItemState, std::logic_error);

class Item :
        public Libshit::RefCounted, public Dumpable, public ParentListBaseHook<>
{
    LIBSHIT_LUA_CLASS;
protected:
    struct Key {};
public:
    // do not change Context& to Weak/Shared ptr
    // otherwise Context's constructor will try to construct a WeakPtr before
    // RefCounted's constructor is finished, making an off-by-one error and
    // freeing the context twice
    explicit Item(
        Key, Context& ctx, FilePosition position = 0) noexcept
        : position{position}, context{&ctx} {}
    Item(const Item&) = delete;
    void operator=(const Item&) = delete;
    virtual ~Item();

    Libshit::RefCountedPtr<Context> GetContextMaybe() noexcept
    { return context.lock(); }
    Libshit::NotNull<Libshit::RefCountedPtr<Context>> GetContext()
    { return Libshit::NotNull<Libshit::RefCountedPtr<Context>>{context}; }
    LIBSHIT_NOLUA Context& GetUnsafeContext() noexcept
    { return *context.unsafe_get(); }
    ItemWithChildren* GetParent() noexcept;

    LIBSHIT_NOLUA Libshit::RefCountedPtr<const Context>
    GetContextMaybe() const noexcept
    { return context.lock(); }
    LIBSHIT_NOLUA Libshit::NotNull<Libshit::RefCountedPtr<const Context>>
    GetContext() const
    { return Libshit::NotNull<Libshit::RefCountedPtr<const Context>>{context}; }
    LIBSHIT_NOLUA const Context& GetUnsafeContext() const noexcept
    { return *context.unsafe_get(); }
    LIBSHIT_NOLUA const ItemWithChildren* GetParent() const noexcept;
    LIBSHIT_NOLUA auto Iterator() const noexcept;
    LIBSHIT_NOLUA auto Iterator() noexcept;

    FilePosition GetPosition() const noexcept { return position; }

    template <typename Checker = Libshit::Check::Assert>
    void Replace(const Libshit::NotNull<Libshit::RefCountedPtr<Item>>& nitem)
    {
        LIBSHIT_CHECK(InvalidItemState, GetParent(), "no parent");
        if constexpr (!Checker::IS_NOP)
        {
            auto nsize = nitem->GetSize();
            for (auto& l : labels)
                LIBSHIT_CHECK(InvalidItemState, l.GetPtr().offset <= nsize,
                               "would invalidate labels");
        }
        LIBSHIT_CHECK(InvalidItemState, nitem->labels.empty(),
                       "new item has labels");
        Replace_(nitem);
    }

    // properties needed: none (might help if ordered)
    // update Slice if no longer ordered
    using LabelsContainer = boost::intrusive::multiset<
        Label,
        boost::intrusive::base_hook<LabelOffsetHook>,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::key_of_value<LabelOffsetKeyOfValue>>;
    LIBSHIT_LUAGEN(wrap="TableRetWrap")
    const LabelsContainer& GetLabels() const { return labels; }

    void Dispose() noexcept override;

protected:
    void UpdatePosition(FilePosition npos);

    void Inspect_(std::ostream& os, unsigned indent) const override = 0;

    using SlicePair = std::pair<Libshit::NotNull<
        Libshit::RefCountedPtr<Item>>, FilePosition>;
    using SliceSeq = std::vector<SlicePair>;
    void Slice(SliceSeq seq);

    FilePosition position;

private:
    Libshit::WeakRefCountedPtr<Context> context;

    LabelsContainer labels;

    void Replace_(const Libshit::NotNull<Libshit::RefCountedPtr<Item>>& nitem);
    virtual void Removed();

    friend class Context;
    friend struct ItemListTraits;
    friend class ItemWithChildren;
    template <typename, typename> friend struct Libshit::Lua::TypeRegisterTraits;
};
static_assert(
    std::is_same_v<Libshit::SmartPtr<Item>, Libshit::RefCountedPtr<Item>>);

std::ostream& operator<<(std::ostream& os, const Item& item);
inline FilePosition ToFilePos(ItemPointer ptr) noexcept
{ return ptr.item->GetPosition() + ptr.offset; }

using ItemList = ParentList<Item, ItemListTraits>;
struct ItemListTraits
{
    static void add(ItemList&, Item& item) noexcept
    { item.AddRef(); }
    static void remove(ItemList&, Item& item) noexcept
    { item.Removed(); item.RemoveRef(); }
};

inline auto Item::Iterator() const noexcept
{ return ItemList::s_iterator_to(*this); }
inline auto Item::Iterator() noexcept
{ return ItemList::s_iterator_to(*this); }

class ItemWithChildren : public Item, private ItemList
{
    LIBSHIT_LUA_CLASS;
public:
    using Item::Item;

    LIBSHIT_LUAGEN(wrap="OwnedSharedPtrWrap")
    ItemList& GetChildren() noexcept { return *this; }
    LIBSHIT_NOLUA const ItemList& GetChildren() const noexcept { return *this; }

    FilePosition GetSize() const override;
    void Fixup() override { Fixup_(0); }

    LIBSHIT_NOLUA void MoveNextToChild(size_t size) noexcept;

    void Dispose() noexcept override;

protected:
    void Dump_(Sink& sink) const override;
    void InspectChildren(std::ostream& sink, unsigned indent) const;
    void Fixup_(FilePosition offset);

private:
    void Removed() override;

    friend struct ::Neptools::ItemListTraits;
    friend class Item;
} LIBSHIT_LUAGEN(post_register=[[
    LIBSHIT_LUA_RUNBC(bld, builder, 1);
    bld.SetField("build");
]]);

inline ItemWithChildren* Item::GetParent() noexcept
{ return static_cast<ItemWithChildren*>(ItemList::opt_get_parent(*this)); }
inline const ItemWithChildren* Item::GetParent() const noexcept
{ return static_cast<const ItemWithChildren*>(ItemList::opt_get_parent(*this)); }

}

template <typename T>
struct Libshit::Lua::SmartObjectMaker<T, std::enable_if_t<
    std::is_base_of_v<Neptools::Item, T> && !std::is_base_of_v<Neptools::Context, T>>>
{
    template <typename Key, typename Ctx, typename... Args>
    static decltype(auto) Make(std::remove_pointer_t<Ctx>& ctx, Args&&... args)
    { return ctx.template Create<T>(std::forward<Args>(args)...); }
};

template<> struct Libshit::Lua::TypeTraits<Neptools::Item::Key>
{
    // HACK
    // Dummy function, needed by LuaGetRef to get above maker to work when
    // using binding generator. It's undefined on purpose.
    template <bool> static void Get(Lua::StateRef, bool, int);
};

#endif
