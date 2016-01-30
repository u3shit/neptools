#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#pragma once

#include "item_base.hpp"
#include <memory>
#include <string>
#include <map>
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
    const Label* CreateLabel(std::string name, ItemPointer ptr);
    const Label* CreateLabelFallback(const std::string& name, ItemPointer ptr);
    const Label* CreateLabelFallback(const std::string& name, FilePosition pos)
    { return CreateLabelFallback(name, GetPointer(pos)); }

    const Label* GetLabelTo(ItemPointer ptr);
    const Label* GetLabelTo(FilePosition pos) { return GetLabelTo(GetPointer(pos)); }

    ItemPointer GetPointer(FilePosition pos) const noexcept;

    void Dump(std::ostream& os) const;

    // properties needed: sorted
    using PointerMap = std::map<FilePosition, Item*>;

protected:
    void SetRoot(std::unique_ptr<Item> nroot);
    ~Context() = default;

private:
    friend class Item;
    size_t size;

    // properties needed: stable pointers
    using LabelsMap = std::unordered_map<std::string, ItemPointer>;
    LabelsMap labels;
    PointerMap pmap;

    // can use stuff inside context
    std::unique_ptr<Item> root;

    const Label* PostCreateLabel(
        std::pair<LabelsMap::iterator, bool> pair, ItemPointer ptr);
};

std::ostream& operator<<(std::ostream& os, const Context& ctx);

#include "item.hpp"
template <typename T, typename... Args>
inline std::unique_ptr<T> Context::Create(Args&&... args)
{
    return std::unique_ptr<T>(new T(Item::Key{}, this, std::forward<Args>(args)...));
}

#endif
