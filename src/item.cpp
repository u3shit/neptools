#include "context.hpp"
#include "item.hpp"
#include "utils.hpp"
#include <boost/assert.hpp>
#include <iostream>

void Item::PrependChild(std::unique_ptr<Item> nitem) noexcept
{
    BOOST_ASSERT(nitem->ctx == ctx && nitem->parent == nullptr &&
                 nitem->prev == nullptr && nitem->next == nullptr);
    nitem->next = std::move(children);
    children = std::move(nitem);
}

void Item::InsertAfter(std::unique_ptr<Item> nitem) noexcept
{
    BOOST_ASSERT(nitem->ctx == ctx && nitem->parent == nullptr &&
                 nitem->prev == nullptr && nitem->next == nullptr);
    nitem->ctx = ctx;
    nitem->parent = parent;

    if (next) next->prev = nitem.get();
    nitem->next = std::move(next);
    nitem->prev = this;
    next = std::move(nitem);
}

void Item::InsertBefore(std::unique_ptr<Item> nitem) noexcept
{
    BOOST_ASSERT(nitem->ctx == ctx && nitem->parent == nullptr &&
                 nitem->prev == nullptr && nitem->next == nullptr);
    auto sav = nitem.get();
    sav->parent = parent;
    sav->ctx = ctx;

    sav->prev = prev;
    std::unique_ptr<Item> thisptr;
    if (prev)
    {
        BOOST_ASSERT(prev->next.get() == this);
        thisptr = std::move(prev->next);
        prev->next = std::move(nitem);
    }
    else if (parent)
    {
        BOOST_ASSERT(parent->children.get() == this);
        thisptr = std::move(parent->children);
        parent->children = std::move(nitem);
    }
    else
    {
        BOOST_ASSERT(ctx->root.get() == this);
        thisptr = std::move(ctx->root);
        ctx->root = std::move(nitem);
    }
    sav->next = std::move(thisptr);
    prev = sav;
}

void Item::Remove() noexcept
{
    BOOST_ASSERT(labels.empty());
    if (next) next->prev = prev;

    if (prev) prev->next = std::move(next);
    else if (parent && parent->children.get() == this)
        parent->children = std::move(next);
    else BOOST_ASSERT(false);
}

void Item::Replace(std::unique_ptr<Item> nitem) noexcept
{
#ifndef BOOST_ASSERT_IS_VOID
    auto nsize = nitem->GetSize();
    for (auto it : labels)
        BOOST_ASSERT(it.first < nsize);
    BOOST_ASSERT(nitem->labels.empty());
#endif

    nitem->labels = std::move(labels);
    labels.clear(); // standard does not guarantee it
    for (auto& it : nitem->labels)
        it.second->second.item = nitem.get();

    InsertAfter(std::move(nitem));
    Remove();
}

void Item::CommitLabels(Key, LabelsContainer&& cnt) noexcept
{
#ifndef BOOST_ASSERT_IS_VOID
    auto size = GetSize();
#endif

    labels = std::move(cnt);
    for (auto& it : labels)
    {
        BOOST_ASSERT(it.first < size);
        auto& ptr = it.second->second;
        ptr.item = this;
        ptr.offset = it.first;
    }
}

void Item::TrimLabels(FilePosition pos) noexcept
{
    EraseIf(labels, [pos](auto x) { return x.first >= pos; });
    MaybeRehash(labels);
}

std::ostream& operator<<(std::ostream& os, const Item& item)
{
    for (auto it = &item; it; it = it->GetNext())
    {
        it->Dump(os);
        os << '\n';
    }
    return os;
}
