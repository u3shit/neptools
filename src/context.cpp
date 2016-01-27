#include "context.hpp"
#include "item.hpp"

Context::Context(std::unique_ptr<Item> root) : root{std::move(root)}
{
    this->root->ctx = this;
}


std::ostream& operator<<(std::ostream& os, const Context& ctx)
{
    if (ctx.GetRoot()) os << *ctx.GetRoot();
    return os;
}
