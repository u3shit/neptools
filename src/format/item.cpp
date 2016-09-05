#include "context.hpp"
#include "item.hpp"
#include "raw_item.hpp"
#include "../utils.hpp"
#include <algorithm>
#include <iostream>

#define NEPTOOLS_LOG_NAME "item"
#include "../logger_helper.hpp"

namespace Neptools
{

Item::~Item()
{
    NEPTOOLS_ASSERT(labels.empty() && parent == nullptr);

    if (ctx)
    {
        auto it = ctx->pmap.find(position);
        if (it != ctx->pmap.end() && it->second == this)
        {
            WARN << "Item " << this << " unlinked from pmap in dtor" << std::endl;
            ctx->pmap.erase(it);
        }
    }
}

void Item::Inspect_(std::ostream& os) const
{
    for (const auto& it : GetLabels())
    {
        os << '@' << it.name;
        if (it.ptr.offset != 0)
            os << '+' << it.ptr.offset;
        os << ":\n";
    }
}

void Item::UpdatePosition(FilePosition npos)
{
    position = npos;
    Fixup();
}

void Item::Replace(NotNull<SmartPtr<Item>> nitem) noexcept
{
#ifndef NDEBUG
    auto nsize = nitem->GetSize();
    for (auto& l : labels)
        NEPTOOLS_ASSERT_MSG(l.ptr.offset <= nsize, "would invalidate labels");
    NEPTOOLS_ASSERT_MSG(nitem->labels.empty(), "new item has labels");
#endif

    // move labels
    nitem->labels.swap(labels); // intrusive move op= does this... (but undocumented)
    for (auto& l : nitem->labels)
        l.ptr.item = nitem.get();

    // update pointermap
    nitem->position = position;
    auto it = ctx->pmap.find(position);
    if (it != ctx->pmap.end() && it->second == this)
        it->second = nitem.get();

    auto& list = parent->GetChildren();
    auto self = Iterator();

    list.insert(self, *nitem);
    list.erase(self);
}

void Item::Slice(SliceSeq seq)
{
    NEPTOOLS_ASSERT(std::is_sorted(seq.begin(), seq.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; }));

    SmartPtr<Item> do_not_delete_this_until_returning{this};

    LabelsContainer lbls{std::move(labels)};
    auto& list = parent->GetChildren();
    auto it = Iterator();
    it = list.erase(it);

    auto& pmap = ctx->pmap;
    auto empty = pmap.empty();
    // remove this from pmap
    if (!empty)
    {
        auto iter = pmap.find(position);
        if (iter != pmap.end() && iter->second == this)
            pmap.erase(iter);
    }

    FilePosition offset = 0;
    auto base_pos = position;

    auto label = lbls.unlink_leftmost_without_rebalance();
    for (auto& el : seq)
    {
        NEPTOOLS_ASSERT(el.first->labels.empty());
        while (label && label->ptr.offset < el.second)
        {
            label->ptr.item = el.first.get();
            label->ptr.offset -= offset;
            el.first->labels.insert(*label);

            label = lbls.unlink_leftmost_without_rebalance();
        }

        // move in place
        el.first->position = base_pos + offset;
        list.insert(it, *el.first);

        // add to pmap if needed
        if (!empty)
            // may throw! but only used during parsing, and an exception there
            // is fatal, so it's not really a problem
            pmap.emplace(el.first->position, &*el.first);

        offset = el.second;
    }
    NEPTOOLS_ASSERT(label == nullptr);
}

std::ostream& operator<<(std::ostream& os, const Item& item)
{
    item.Inspect(os);
    return os;
}

void ItemList::Add(reference it)
{
    auto self = static_cast<ItemWithChildren*>(this);
    NEPTOOLS_ASSERT_MSG(it.ctx == self->ctx, "wrong context");
    NEPTOOLS_ASSERT_MSG(it.parent == nullptr, "already added");
    // not in list: checked by boost::intrusive (but parent check should do it too)
    it.parent = self;
    intrusive_ptr_add_ref(&it);
}

void ItemList::Disposer::operator()(pointer p)
{
    NEPTOOLS_ASSERT(p->parent == static_cast<ItemWithChildren*>(list));
    p->parent = nullptr;
    intrusive_ptr_release(p);
}


void ItemWithChildren::Dump_(Sink& sink) const
{
    for (auto& c : GetChildren())
        c.Dump(sink);
}

void ItemWithChildren::Inspect_(std::ostream& os) const
{
    for (auto& c : GetChildren())
    {
        c.Inspect(os);
        os << '\n';
    }
}


FilePosition ItemWithChildren::GetSize() const
{
    FilePosition ret = 0;
    for (auto& c : GetChildren())
        ret += c.GetSize();
    return ret;
}

void ItemWithChildren::Fixup_(FilePosition offset)
{
    FilePosition pos = position + offset;
    for (auto& c : GetChildren())
    {
        c.UpdatePosition(pos);
        pos += c.GetSize();
    }
}

void ItemWithChildren::MoveNextToChild(size_t size) noexcept
{
    auto& list = GetParent().GetChildren();
    // make sure we have a ref when erasing...
    SmartPtr<Item> nchild = &asserted_cast<RawItem&>(
        *++Iterator()).Split(0, size);
    list.erase(nchild->Iterator());
    GetChildren().push_back(*nchild);
}

}
