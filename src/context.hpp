#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#pragma once

#include <memory>
class Item;

class Context
{
public:
    Context(std::unique_ptr<Item> root);

    Item* GetRoot() noexcept { return root.get(); }
    const Item* GetRoot() const noexcept { return root.get(); }
private:
    friend class Item;
    std::unique_ptr<Item> root;
};

std::ostream& operator<<(std::ostream& os, const Context& ctx);

#endif
