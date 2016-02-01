#include "context.hpp"
#include "item.hpp"
#include "utils.hpp"
#include <boost/assert.hpp>
#include <algorithm>
#include <iostream>

Item::~Item()
{
    //BOOST_ASSERT(labels.empty());
    auto it = ctx->pmap.find(position);
    if (it != ctx->pmap.end() && it->second == this)
        ctx->pmap.erase(it);
}

void Item::PrettyPrint(std::ostream& os) const
{
    for (const auto& it : GetLabels())
    {
        os << '@' << it.second->first;
        if (it.first != 0)
            os << '+' << it.first;
        os << ":\n";
    }
}

size_t Item::UpdatePositions(FilePosition npos)
{
    position = npos;
    if (GetNext())
        return GetNext()->UpdatePositions(npos + GetSize());
    else
        return npos + GetSize();
}

void Item::PrependChild(std::unique_ptr<Item> nitem) noexcept
{
    BOOST_ASSERT(nitem->ctx == ctx && nitem->parent == nullptr &&
                 nitem->prev == nullptr && nitem->next == nullptr);
    nitem->next = std::move(children);
    nitem->parent = this;
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

std::unique_ptr<Item> Item::Remove() noexcept
{
    if (next) next->prev = prev;

    std::unique_ptr<Item> ret;
    if (prev)
    {
        BOOST_ASSERT(prev->next.get() == this);
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
        BOOST_ASSERT(ctx->root.get() == this);
        ret = std::move(ctx->root);
        ctx->root = std::move(next);
    }

    prev = parent = nullptr;
    return ret;
}

void Item::Replace(std::unique_ptr<Item> nitem) noexcept
{
#ifndef BOOST_ASSERT_IS_VOID
    auto nsize = nitem->GetSize();
    for (auto it : labels)
        BOOST_ASSERT(it.first < nsize);
    BOOST_ASSERT(nitem->labels.empty());
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
    BOOST_ASSERT(std::is_sorted(seq.begin(), seq.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; }));
    BOOST_ASSERT(seq[0].second == 0);

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
    BOOST_ASSERT(moved == 1);
}

void Item::CommitLabels(LabelsContainer&& cnt) noexcept
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

std::ostream& operator<<(std::ostream& os, const Item& item)
{
    for (auto it = &item; it; it = it->GetNext())
    {
        it->PrettyPrint(os);
        os << '\n';
    }
    return os;
}

void ItemWithChildren::Dump(std::ostream& os) const
{
    for (auto it = GetChildren(); it; it = it->GetNext())
        it->Dump(os);
}

size_t ItemWithChildren::GetSize() const noexcept
{
    size_t ret = 0;
    for (auto it = GetChildren(); it; it = it->GetNext())
        ret += it->GetSize();
    return ret;
}

size_t ItemWithChildren::UpdatePositions(FilePosition npos)
{
    if (GetChildren())
        GetChildren()->UpdatePositions(npos);
    return Item::UpdatePositions(npos);
}
