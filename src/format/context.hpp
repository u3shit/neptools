#ifndef UUID_C2DF26B7_DE7D_47B8_BAEE_9F6EEBB12891
#define UUID_C2DF26B7_DE7D_47B8_BAEE_9F6EEBB12891
#pragma once

#include "item.hpp"
#include "../dumpable.hpp"

#include <boost/exception/info.hpp>
#include <boost/intrusive/set.hpp>
#include <string>
#include <map>

namespace Neptools
{

class Context : public ItemWithChildren
{
    NEPTOOLS_LUA_CLASS;
public:
    Context();
    ~Context();

    void Fixup() override;

    template <typename T, typename... Args>
    NEPTOOLS_NOLUA NotNull<SmartPtr<T>> Create(Args&&... args)
    { return MakeSmart<T>(Item::Key{}, *this, std::forward<Args>(args)...); }

    NotNull<LabelPtr> GetLabel(const std::string& name) const;
    NotNull<LabelPtr> CreateLabel(std::string name, ItemPointer ptr);
    NotNull<LabelPtr> CreateLabelFallback(std::string name, ItemPointer ptr);
    NotNull<LabelPtr> CreateLabelFallback(std::string name, FilePosition pos)
    { return CreateLabelFallback(std::move(name), GetPointer(pos)); }

    NotNull<LabelPtr> GetLabelTo(ItemPointer ptr);
    NotNull<LabelPtr> GetLabelTo(FilePosition pos)
    { return GetLabelTo(GetPointer(pos)); }

    NotNull<LabelPtr> GetLabelTo(FilePosition pos, std::string name);

    ItemPointer GetPointer(FilePosition pos) const noexcept;

    void Dispose() noexcept override;

protected:
    void SetupParseFrom(Item& item);

private:
    static void FilterLabelName(std::string& name);

    friend class Item;

    // properties needed: stable pointers
    using LabelsMap = boost::intrusive::set<
        Label,
        boost::intrusive::base_hook<LabelNameHook>,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::key_of_value<LabelKeyOfValue>>;
    LabelsMap labels;

    // properties needed: sorted
    using PointerMap = std::map<FilePosition, Item*>;
    PointerMap pmap;
};

using AffectedLabel = boost::error_info<struct AffectedLabelTag, std::string>;
}

#endif
