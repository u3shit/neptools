#ifndef UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#define UUID_294A7F35_D9EC_4A07_961F_A74307C4FA29
#pragma once

#include "item_base.hpp"
#include "../dumpable.hpp"
#include <iosfwd>
#include <vector>
#include <map>

class Item : public Dumpable
{
protected:
    struct Key {};
public:
    explicit Item(Key, Context* ctx, FilePosition position = 0) noexcept
        : position{position}, ctx{ctx} {}
    Item(const Item&) = delete;
    void operator=(const Item&) = delete;
    virtual ~Item();

    FilePosition UpdatePositions(FilePosition npos);

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
    std::unique_ptr<Item> RemoveChildren() noexcept
    { return std::move(children); }
    void Replace(std::unique_ptr<Item> nitem) noexcept;

    // properties needed: none (might help if ordered)
    using LabelsContainer = std::multimap<FilePosition, Label*>;
    const LabelsContainer& GetLabels() const { return labels; }

protected:
    void Inspect_(std::ostream& os) const override = 0;

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

class ItemWithChildren : public Item
{
public:
    using Item::Item;

    FilePosition GetSize() const noexcept override;
    void Fixup() override;

protected:
    void Dump_(Sink& sink) const override;
};

#endif
