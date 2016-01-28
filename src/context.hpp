#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#pragma once

#include "item_base.hpp"
#include <memory>
#include <string>
#include <unordered_map>

class Context
{
public:
    Item* GetRoot() noexcept { return root.get(); }
    const Item* GetRoot() const noexcept { return root.get(); }

    size_t GetSize() const noexcept { return size; }

    template <typename T, typename... Args>
    std::unique_ptr<T> Create(Args&&... args);

    ItemPointer GetLabel(const std::string& name) const { return labels.at(name); }
    const Label* CreateLabel(const std::string& name, ItemPointer ptr);
    const Label* GetLabelTo(ItemPointer ptr);

protected:
    void SetRoot(std::unique_ptr<Item> nroot);
    ~Context() = default;

private:
    friend class Item;
    std::unique_ptr<Item> root;
    size_t size;

    // properties needed: stable pointers
    std::unordered_map<std::string, ItemPointer> labels;
};

std::ostream& operator<<(std::ostream& os, const Context& ctx);

#include "item.hpp"
template <typename T, typename... Args>
inline std::unique_ptr<T> Context::Create(Args&&... args)
{
    return std::unique_ptr<T>(new T(Item::Key{}, this, std::forward<Args>(args)...));
}

#endif
