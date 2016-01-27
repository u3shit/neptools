#ifndef ITEM_HPP
#define ITEM_HPP
#pragma once

#include "buffer.hpp"
#include <cstdint>
#include <iostream>

class Item;
class Context;
using FilePointer = std::uint32_t;

class Item
{
public:
    explicit Item(FilePointer position = 0) noexcept : position{position} {}
    Item(const Item&) = delete;
    void operator=(const Item&) = delete;
    virtual ~Item() = default;

    virtual void Dump(std::ostream& os) const = 0;
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

    FilePointer GetPosition() const noexcept { return position; }

    void PrependChild(std::unique_ptr<Item> nitem) noexcept;
    void InsertAfter(std::unique_ptr<Item> nitem) noexcept;
    void InsertBefore(std::unique_ptr<Item> nitem) noexcept;
    void Remove() noexcept;
    void Replace(std::unique_ptr<Item> nitem) noexcept;

protected:
    FilePointer position = 0;

private:
    friend class Context;
    Context* ctx = nullptr;
    Item* parent = nullptr;
    std::unique_ptr<Item> children;
    Item* prev = nullptr;
    std::unique_ptr<Item> next;
};

inline std::ostream& operator<<(std::ostream& os, const Item& item)
{
    for (auto it = &item; it; it = it->GetNext())
    {
        it->Dump(os);
        os << '\n';
    }
    return os;
}

#endif
