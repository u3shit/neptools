#include "context.hpp"
#include "item.hpp"
#include "../utils.hpp"
#include "../except.hpp"
#include <iomanip>
#include <fstream>
#include <sstream>

namespace Neptools
{

Context::Context()
    : ItemWithChildren{Key{}, this}
{}

Context::~Context()
{
    pmap.clear();
    struct Disposer
    {
        void operator()(Label* l)
        {
#ifndef NDEBUG
            auto& lst = l->ptr.item->labels;
            lst.erase(lst.iterator_to(*l));
#endif
            delete l;
        }
    };
    labels.clear_and_dispose(Disposer{});
    GetChildren().clear();
    ctx = nullptr;
}

void Context::SetupParseFrom(Item& item)
{
    pmap[0] = &item;
    GetChildren().push_back(item); // noexcept
}

void Context::Fixup()
{
    pmap.clear();

    FilePosition pos = 0;
    for (auto& c : GetChildren())
    {
        c.UpdatePosition(pos);
        pos += c.GetSize();
    }
    // todo? size = pos;
}


const Label& Context::GetLabel(const std::string& name) const
{
    auto it = labels.find(name);
    if (it == labels.end())
        NEPTOOLS_THROW(OutOfRange{"Context::GetLabel"}
                       << AffectedLabel{name});
    return *it;
}

const Label& Context::CreateLabel(std::string name, ItemPointer ptr)
{
    FilterLabelName(name);
    auto lbl = new Label{std::move(name), ptr};
    auto pair = labels.insert(*lbl);
    if (!pair.second)
    {
        name = std::move(lbl->name);
        delete lbl;
        NEPTOOLS_THROW(OutOfRange{"label already exists"}
                       << AffectedLabel{std::move(name)});
    }

    ptr->labels.insert(*pair.first);
    return *pair.first;
}

const Label& Context::CreateLabelFallback(std::string name, ItemPointer ptr)
{
    FilterLabelName(name);
    LabelsMap::insert_commit_data commit;
    std::string str = std::move(name);

    auto pair = labels.insert_check(str, commit);
    for (int i = 1; !pair.second; ++i)
    {
        std::stringstream ss;
        ss << name << '_' << i;
        str = ss.str();
        pair = labels.insert_check(str, commit);
    }

    auto it = labels.insert_commit(*new Label{std::move(str), ptr}, commit);

    ptr->labels.insert(*it);
    return *it;
}

const Label& Context::GetLabelTo(ItemPointer ptr)
{
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return *it;

    std::stringstream ss;
    ss << "loc_" << std::setw(8) << std::setfill('0') << std::hex
       << ptr.item->GetPosition() + ptr.offset;
    return CreateLabelFallback(ss.str(), ptr);
}

const Label& Context::GetLabelTo(FilePosition pos, std::string name)
{
    auto ptr = GetPointer(pos);
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return *it;

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
    NEPTOOLS_ASSERT(it->first == it->second->GetPosition());
    return {it->second, pos - it->first};
}

}
