#ifndef ITEM_HPP
#define ITEM_HPP
#pragma once

#include "item_base.hpp"
#include "buffer.hpp"
#include <iosfwd>
#include <vector>
#include <unordered_map>

class Item
{
protected:
    struct Key {};
public:
    explicit Item(Key, Context* ctx, FilePosition position = 0) noexcept
        : position{position}, ctx{ctx} {}
    Item(const Item&) = delete;
    void operator=(const Item&) = delete;
    virtual ~Item();

    virtual void Dump(std::ostream& os) const = 0;
    virtual void PrettyPrint(std::ostream& os) const = 0;
    virtual size_t GetSize() const noexcept = 0;

    Context* GetContext() noexcept { return ctx; }
    Item* GetParent() noexcept   { return parent; }
    Item* GetChildren() noexcept { return children.get(); }
    Item* GetNext() noexcept     { return next.get(); }
    Item* GetPrev() noexcept     { return prev; }

    const Context* GetContext() const noexcept { return ctx; }
    const Item* GetParent() const noexcept   { return parent; }
    const Item* GetChildren() const noexcept { return children.get(); }
    const Item* GetNext() const noexcept     { return next.get(); }
    const Item* GetPrev() const noexcept     { return prev; }

    FilePosition GetPosition() const noexcept { return position; }

    void PrependChild(std::unique_ptr<Item> nitem) noexcept;
    void InsertAfter(std::unique_ptr<Item> nitem) noexcept;
    void InsertBefore(std::unique_ptr<Item> nitem) noexcept;
    std::unique_ptr<Item> Remove() noexcept;
    void Replace(std::unique_ptr<Item> nitem) noexcept;

    // properties needed: none (might help if ordered)
    using LabelsContainer = std::unordered_multimap<FilePosition, Label*>;
    const LabelsContainer& GetLabels() const { return labels; }

protected:
    using SlicePair = std::pair<std::unique_ptr<Item>, FilePosition>;
    using SliceSeq = std::vector<SlicePair>;
    void Slice(SliceSeq seq);

    FilePosition position;

private:
    void CommitLabels(LabelsContainer&& ctr) noexcept;

    friend class Context;
    Context* ctx;
    Item* parent = nullptr;
    std::unique_ptr<Item> children;
    Item* prev = nullptr;
    std::unique_ptr<Item> next;

    LabelsContainer labels;
};

std::ostream& operator<<(std::ostream& os, const Item& item);
inline FilePosition ToFilePos(ItemPointer ptr) noexcept
{ return ptr.item->GetPosition() + ptr.offset; }

#endif
