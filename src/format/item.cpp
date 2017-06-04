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

void Lua::TypeTraits<Label>::Push(StateRef vm, const Label& l)
{
    NEPTOOLS_LUA_GETTOP(vm, top);
    lua_createtable(vm, 0, 3); // +1
    vm.Push(l.name); // +2
    lua_setfield(vm, -2, "name"); // +1
    vm.Push(l.ptr.item); // +2
    lua_setfield(vm, -2, "item"); // +1
    vm.Push(l.ptr.offset); // +2
    lua_setfield(vm, -2, "offset"); // +1
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}

Item::~Item()
{
    Item::Dispose();
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

void Item::Replace_(const NotNull<SmartPtr<Item>>& nitem) noexcept
{
    auto ctx = GetContext();
    // move labels
    nitem->labels.swap(labels); // intrusive move op= does this... (but undocumented)
    for (auto& l : nitem->labels)
        l.ptr.item = nitem.get();

    // update pointermap
    nitem->position = position;
    auto it = ctx->pmap.find(position);
    if (it != ctx->pmap.end() && it->second == this)
        it->second = nitem.get();

    auto& list = GetParent()->GetChildren();
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
    auto& list = GetParent()->GetChildren();
    auto it = Iterator();
    it = list.erase(it);

    auto& pmap = GetUnsafeContext().pmap;
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

void Item::Dispose() noexcept
{
    NEPTOOLS_ASSERT(labels.empty() && !GetParent());
    if (auto ctx = GetContextMaybe())
    {
        auto it = ctx->pmap.find(position);
        if (it != ctx->pmap.end() && it->second == this)
        {
            WARN << "Item " << this << " unlinked from pmap in Dispose" << std::endl;
            ctx->pmap.erase(it);
        }
    }

    context.reset();
}

std::ostream& operator<<(std::ostream& os, const Item& item)
{
    item.Inspect(os);
    return os;
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
    auto& list = GetParent()->GetChildren();
    // make sure we have a ref when erasing...
    SmartPtr<Item> nchild = &asserted_cast<RawItem&>(
        *++Iterator()).Split(0, size);
    list.erase(nchild->Iterator());
    GetChildren().push_back(*nchild);
}

void ItemWithChildren::Dispose() noexcept
{
    GetChildren().clear();
    Item::Dispose();
}

}

#include "../lua/table_ret_wrap.hpp"
#include "../lua/owned_shared_ptr_wrap.hpp"
#include "item.binding.hpp"
