#include "context.hpp"
#include "item.hpp"
#include "../utils.hpp"
#include "../except.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

namespace Neptools
{

void Context::SetRoot(std::unique_ptr<Item> nroot)
{
    NEPTOOLS_ASSERT_MSG(nroot->ctx == this && nroot->position == 0,
                        "invalid root item");
    PointerMap npmap{{0, nroot.get()}};

    pmap = std::move(npmap);
    root = std::move(nroot);
    size = root->GetSize();
}

const Label* Context::GetLabel(std::string name) const
{
    FilterLabelName(name);
    auto it = labels.find(name);
    if (it == labels.end())
        NEPTOOLS_THROW(OutOfRange{"Context::GetLabel"}
                       << AffectedLabel{name});
    return &*it;
}

const Label* Context::CreateLabel(std::string name, ItemPointer ptr)
{
    FilterLabelName(name);
    auto pair = labels.insert({std::move(name), ptr});
    if (!pair.second)
        NEPTOOLS_THROW(OutOfRange{"label already exists"}
                       << AffectedLabel(name));

    return PostCreateLabel(pair, ptr);
}

const Label* Context::CreateLabelFallback(std::string name, ItemPointer ptr)
{
    FilterLabelName(name);
    auto pair = labels.insert({name, ptr});
    for (int i = 1; !pair.second; ++i)
    {
        std::stringstream ss;
        ss << name << '_' << i;
        pair = labels.insert({ss.str(), ptr});
    }
    return PostCreateLabel(pair, ptr);
}

const Label* Context::PostCreateLabel(
    std::pair<LabelsMap::iterator, bool> pair, ItemPointer ptr)
{
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
    return item;
}

const Label* Context::GetLabelTo(ItemPointer ptr)
{
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return it->second;

    std::stringstream ss;
    ss << "loc_" << std::setw(8) << std::setfill('0') << std::hex
       << ptr.item->GetPosition() + ptr.offset;
    return CreateLabelFallback(ss.str(), ptr);
}

const Label* Context::GetLabelTo(FilePosition pos, std::string name)
{
    auto ptr = GetPointer(pos);
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return it->second;

    return CreateLabelFallback(std::move(name), ptr);
}

void Context::FilterLabelName(std::string& name)
{
    for (auto& c : name)
        if (!isalnum(c)) c = '_';
}

ItemPointer Context::GetPointer(FilePosition pos) const noexcept
{
    auto it = pmap.upper_bound(pos);
    NEPTOOLS_ASSERT_MSG(it != pmap.begin(), "file position out of range");
    --it;
    return {it->second, pos - it->first};
}

void Context::UpdatePositions()
{
    pmap.clear();
    if (GetRoot())
        size = GetRoot()->UpdatePositions(0);
    else
        size = 0;
}

void Context::Dump_(Sink& sink) const
{
    for (auto it = GetRoot(); it; it = it->GetNext())
        it->Dump(sink);
}

void Context::Inspect_(std::ostream& os) const
{
    if (GetRoot()) os << *GetRoot();
}

}
