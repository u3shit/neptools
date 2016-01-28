#include "context.hpp"
#include "item.hpp"
#include <boost/assert.hpp>
#include <iomanip>
#include <sstream>

void Context::SetRoot(std::unique_ptr<Item> nroot)
{
    BOOST_ASSERT(nroot->ctx == this);
    root = std::move(nroot);
    size = root->GetSize();
}

const Label& Context::CreateLabel(const std::string& name, ItemPointer ptr)
{
    auto pair = labels.insert({name, ptr});
    if (!pair.second)
        throw std::out_of_range("label already exists");

    Label* item = &*pair.first;
    try
    {
        ptr.item->labels.insert({ptr.offset, item});
    }
    catch (...)
    {
        labels.erase(pair.first);
        throw;
    }
    return *item;
}

const Label& Context::GetLabelTo(ItemPointer ptr)
{
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return *it->second;

    std::stringstream ss;
    ss << "loc_" << std::setw(8) << std::setfill('0') << std::hex
       << ptr.item->GetPosition() + ptr.offset;
    return CreateLabel(ss.str(), ptr); // todo: duplicates?!
}

std::ostream& operator<<(std::ostream& os, const Context& ctx)
{
    if (ctx.GetRoot()) os << *ctx.GetRoot();
    return os;
}
