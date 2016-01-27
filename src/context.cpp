#include "context.hpp"
#include "item.hpp"
#include <boost/assert.hpp>

void Context::SetRoot(std::unique_ptr<Item> nroot)
{
    BOOST_ASSERT(nroot->ctx == this);
    root = std::move(nroot);
    size = root->GetSize();
}


std::ostream& operator<<(std::ostream& os, const Context& ctx)
{
    if (ctx.GetRoot()) os << *ctx.GetRoot();
    return os;
}
