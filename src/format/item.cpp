#include "context.hpp"
#include "item.hpp"
#include "../utils.hpp"
#include <algorithm>
#include <iostream>

namespace Neptools
{

Item::~Item()
{
    //BOOST_ASSERT(labels.empty());
    auto it = ctx->pmap.find(position);
    if (it != ctx->pmap.end() && it->second == this)
        ctx->pmap.erase(it);
}

void Item::Inspect_(std::ostream& os) const
{
    for (const auto& it : GetLabels())
    {
        os << '@' << it.second->first;
        if (it.first != 0)
            os << '+' << it.first;
        os << ":\n";
    }
}

FilePosition Item::UpdatePositions(FilePosition npos)
{
    position = npos;
    Fixup();
    if (GetNext())
        return GetNext()->UpdatePositions(npos + GetSize());
    else
        return npos + GetSize();
}

void Item::PrependChild(std::unique_ptr<Item> nitem) noexcept
{
    NEPTOOLS_ASSERT_MSG(
        nitem->ctx == ctx && nitem->parent == nullptr &&
        nitem->prev == nullptr && nitem->next == nullptr,
        "item already added?");
    nitem->next = std::move(children);
    nitem->parent = this;
    children = std::move(nitem);
}

void Item::InsertAfter(std::unique_ptr<Item> nitem) noexcept
{
    NEPTOOLS_ASSERT_MSG(
        nitem->ctx == ctx && nitem->parent == nullptr &&
        nitem->prev == nullptr && nitem->next == nullptr,
        "item already added?");
    nitem->ctx = ctx;
    nitem->parent = parent;

    if (next) next->prev = nitem.get();
    nitem->next = std::move(next);
    nitem->prev = this;
    next = std::move(nitem);
}

void Item::InsertBefore(std::unique_ptr<Item> nitem) noexcept
{
    NEPTOOLS_ASSERT_MSG(
        nitem->ctx == ctx && nitem->parent == nullptr &&
        nitem->prev == nullptr && nitem->next == nullptr,
        "item already added?");
    auto sav = nitem.get();
    sav->parent = parent;
    sav->ctx = ctx;

    sav->prev = prev;
    std::unique_ptr<Item> thisptr;
    if (prev)
    {
        NEPTOOLS_ASSERT(prev->next.get() == this);
        thisptr = std::move(prev->next);
        prev->next = std::move(nitem);
    }
    else if (parent)
    {
        NEPTOOLS_ASSERT(parent->children.get() == this);
        thisptr = std::move(parent->children);
        parent->children = std::move(nitem);
    }
    else
    {
        NEPTOOLS_ASSERT(ctx->root.get() == this);
        thisptr = std::move(ctx->root);
        ctx->root = std::move(nitem);
    }
    sav->next = std::move(thisptr);
    prev = sav;
}

std::unique_ptr<Item> Item::Remove() noexcept
{
    if (next) next->prev = prev;

    std::unique_ptr<Item> ret;
    if (prev)
    {
        NEPTOOLS_ASSERT(prev->next.get() == this);
        ret = std::move(prev->next);
        prev->next = std::move(next);
    }
    else if (parent && parent->children.get() == this)
    {
        ret = std::move(parent->children);
        parent->children = std::move(next);
    }
    else
    {
        NEPTOOLS_ASSERT_MSG(ctx->root.get() == this, "item already removed?");
        ret = std::move(ctx->root);
        ctx->root = std::move(next);
    }

    prev = parent = nullptr;
    return ret;
}

void Item::Replace(std::unique_ptr<Item> nitem) noexcept
{
#ifndef NDEBUG
    auto nsize = nitem->GetSize();
    for (auto it : labels)
        NEPTOOLS_ASSERT_MSG(it.first <= nsize, "would invalidate labels");
    NEPTOOLS_ASSERT_MSG(nitem->labels.empty(), "new item has labels");
#endif

    // move labels
    nitem->labels = std::move(labels);
    labels.clear(); // standard does not guarantee it
    for (auto& it : nitem->labels)
        it.second->second.item = nitem.get();

    // update pointermap
    nitem->position = position;
    auto it = ctx->pmap.find(position);
    if (it != ctx->pmap.end()) it->second = nitem.get();

    InsertAfter(std::move(nitem));
    Remove();
}

namespace
{
struct PmapRem
{
    using T = std::pair<Context::PointerMap::iterator, bool>;
    Context::PointerMap& pmap;
    T x;

    PmapRem(Context::PointerMap& pmap, T x) noexcept : pmap(pmap), x(x) {}
    PmapRem(PmapRem&& o) noexcept : pmap(o.pmap), x(o.x) { o.x.second = false; }
    void operator=(const PmapRem&) = delete;
    ~PmapRem() { if (x.second) pmap.erase(x.first); }
};
}

void Item::Slice(SliceSeq seq)
{
    NEPTOOLS_ASSERT(std::is_sorted(seq.begin(), seq.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; }));
    NEPTOOLS_ASSERT(seq[0].second == 0);

    std::vector<PmapRem> ps;
    std::vector<LabelsContainer> nlabels;
    ps.reserve(seq.size());
    nlabels.resize(seq.size());

    for (const auto& p : seq)
        ps.emplace_back(ctx->pmap, ctx->pmap.insert({position + p.second, {}}));


    for (auto lbl : GetLabels())
    {
        size_t i = seq.size() - 1;
        for (; lbl.first < seq[i].second; --i);
        nlabels[i].insert({lbl.first - seq[i].second, lbl.second});
    }

    // everything's allocd, commit it....
    auto prev = this;
    auto base_pos = position;
    size_t moved = 0;
    for (size_t i = 0; i < seq.size(); ++i)
    {
        auto cur = seq[i].first.get();
        if (cur == nullptr) cur = this;
        ps[i].x.first->second = cur;
        ps[i].x.second = false;

        cur->position = base_pos + seq[i].second;
        cur->CommitLabels(std::move(nlabels[i]));
        if (seq[i].first == nullptr)
        {
            if (prev != this) prev->InsertAfter(Remove());
            ++moved;
        }
        else
            prev->InsertAfter(std::move(seq[i].first));
        prev = cur;
    }
    NEPTOOLS_ASSERT_MSG(
        moved == 1, "invalid seq: zero or more than one nullptrs");
}

void Item::CommitLabels(LabelsContainer&& cnt) noexcept
{
#ifndef NDEBUG
    auto size = GetSize();
#endif

    labels = std::move(cnt);
    for (auto& it : labels)
    {
#ifndef NDEBUG // otherwise __assume fails
        NEPTOOLS_ASSERT(it.first < size);
#endif
        auto& ptr = it.second->second;
        ptr.item = this;
        ptr.offset = it.first;
    }
}

std::ostream& operator<<(std::ostream& os, const Item& item)
{
    for (auto it = &item; it; it = it->GetNext())
    {
        it->Inspect(os);
        os << '\n';
    }
    return os;
}

void ItemWithChildren::Dump_(Sink& sink) const
{
    for (auto it = GetChildren(); it; it = it->GetNext())
        it->Dump(sink);
}

FilePosition ItemWithChildren::GetSize() const noexcept
{
    FilePosition ret = 0;
    for (auto it = GetChildren(); it; it = it->GetNext())
        ret += it->GetSize();
    return ret;
}

void ItemWithChildren::Fixup()
{
    if (GetChildren())
        GetChildren()->UpdatePositions(position);
}

}
