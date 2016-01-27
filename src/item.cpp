#include "context.hpp"
#include "item.hpp"
#include <cassert>

void Item::PrependChild(std::unique_ptr<Item> nitem) noexcept
{
    assert(nitem->parent == nullptr && nitem->prev == nullptr &&
           nitem->next == nullptr);
    nitem->next = std::move(children);
    children = std::move(nitem);
}

void Item::InsertAfter(std::unique_ptr<Item> nitem) noexcept
{
    assert(nitem->ctx == nullptr && nitem->parent == nullptr &&
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
    assert(nitem->ctx == nullptr && nitem->parent == nullptr &&
           nitem->prev == nullptr && nitem->next == nullptr);
    auto sav = nitem.get();
    sav->parent = parent;
    sav->ctx = ctx;

    sav->prev = prev;
    std::unique_ptr<Item> thisptr;
    if (prev)
    {
        assert(prev->next.get() == this);
        thisptr = std::move(prev->next);
        prev->next = std::move(nitem);
    }
    else if (parent)
    {
        assert(parent->children.get() == this);
        thisptr = std::move(parent->children);
        parent->children = std::move(nitem);
    }
    else
    {
        assert(ctx->root.get() == this);
        thisptr = std::move(ctx->root);
        ctx->root = std::move(nitem);
    }
    sav->next = std::move(thisptr);
    prev = sav;
}

void Item::Remove() noexcept
{
    if (next) next->prev = prev;

    if (prev) prev->next = std::move(next);
    else if (parent && parent->children.get() == this)
        parent->children = std::move(next);
    else assert(false);
}

void Item::Replace(std::unique_ptr<Item> nitem) noexcept
{
    InsertAfter(std::move(nitem));
    Remove();
}
