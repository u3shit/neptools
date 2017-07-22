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
    : ItemWithChildren{Key{}, *this}
{}

Context::~Context()
{
    Context::Dispose();
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


NotNull<LabelPtr> Context::GetLabel(const std::string& name) const
{
    auto it = labels.find(name);
    if (it == labels.end())
        NEPTOOLS_THROW(OutOfRange{"Context::GetLabel"}
                       << AffectedLabel{name});
    return MakeNotNull(const_cast<Label*>(&*it));
}

NotNull<LabelPtr> Context::CreateLabel(std::string name, ItemPointer ptr)
{
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
    return MakeNotNull(&*pair.first);
}

NotNull<LabelPtr> Context::CreateLabelFallback(
    const std::string& name, ItemPointer ptr)
{
    LabelsMap::insert_commit_data commit;
    std::string str = name;

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
    return MakeNotNull(&*it);
}

NotNull<LabelPtr> Context::CreateOrSetLabel(std::string name, ItemPointer ptr)
{
    LabelsMap::insert_commit_data commit;
    auto [it, insertable] = labels.insert_check(name, commit);

    if (insertable)
    {
        auto it = labels.insert_commit(*new Label{std::move(name), ptr}, commit);
        ptr->labels.insert(*it);
        return MakeNotNull(&*it);
    }
    else
    {
        if (it->ptr != nullptr) it->ptr->labels.remove_node(*it);
        it->ptr = ptr;
        ptr->labels.insert(*it);
        return MakeNotNull(&*it);
    }
}

NotNull<LabelPtr> Context::GetOrCreateDummyLabel(std::string name)
{
    LabelsMap::insert_commit_data commit;
    auto [it, insertable] = labels.insert_check(name, commit);

    if (insertable)
        it = labels.insert_commit(
            *new Label{std::move(name), {nullptr,0}}, commit);
    return MakeNotNull(const_cast<Label*>(&*it));
}

NotNull<LabelPtr> Context::GetLabelTo(ItemPointer ptr)
{
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return MakeNotNull(&*it);

    std::stringstream ss;
    ss << "loc_" << std::setw(8) << std::setfill('0') << std::hex
       << ptr.item->GetPosition() + ptr.offset;
    return CreateLabelFallback(ss.str(), ptr);
}

NotNull<LabelPtr> Context::GetLabelTo(FilePosition pos, std::string name)
{
    auto ptr = GetPointer(pos);
    auto& lctr = ptr.item->labels;
    auto it = lctr.find(ptr.offset);
    if (it != lctr.end()) return MakeNotNull(&*it);

    return CreateLabelFallback(std::move(name), ptr);
}

ItemPointer Context::GetPointer(FilePosition pos) const noexcept
{
    auto it = pmap.upper_bound(pos);
    NEPTOOLS_ASSERT_MSG(it != pmap.begin(), "file position out of range");
    --it;
    NEPTOOLS_ASSERT(it->first == it->second->GetPosition());
    return {it->second, pos - it->first};
}

void Context::Dispose() noexcept
{
    pmap.clear();
    struct Disposer
    {
        void operator()(Label* l)
        {
            auto& item = l->ptr.item;
            if (item)
            {
                item->labels.erase(item->labels.iterator_to(*l));
                item = nullptr;
            }
            l->RemoveRef();
        }
    };
    labels.clear_and_dispose(Disposer{});
    GetChildren().clear();

    ItemWithChildren::Dispose();
}

std::ostream& operator<<(std::ostream& os, PrintLabelStruct l)
{
    if (l.label == nullptr)
        return os << "nil";
    return os << "l(" << Quoted(l.label->GetName()) << ')';
}

}

#include "context.binding.hpp"
