#include "context.hpp"
#include "item.hpp"
#include <cassert>

void Context::SetRoot(std::unique_ptr<Item> nroot)
{
    assert(nroot->ctx == this);
    root = std::move(nroot);
    size = root->GetSize();
}


std::ostream& operator<<(std::ostream& os, const Context& ctx)
{
    if (ctx.GetRoot()) os << *ctx.GetRoot();
    return os;
}
