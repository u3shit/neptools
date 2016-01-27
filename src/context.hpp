#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#pragma once

#include <memory>
class Item;

class Context
{
public:
    Item* GetRoot() noexcept { return root.get(); }
    const Item* GetRoot() const noexcept { return root.get(); }

    size_t GetSize() const noexcept { return size; }

    template <typename T, typename... Args>
    std::unique_ptr<T> Create(Args&&... args);

protected:
    void SetRoot(std::unique_ptr<Item> nroot);
    ~Context() = default;

private:
    friend class Item;
    std::unique_ptr<Item> root;
    size_t size;
};

std::ostream& operator<<(std::ostream& os, const Context& ctx);

#include "item.hpp"
template <typename T, typename... Args>
inline std::unique_ptr<T> Context::Create(Args&&... args)
{
    return std::unique_ptr<T>(new T(Item::ContextKey{this}, std::forward<Args>(args)...));
}

#endif
