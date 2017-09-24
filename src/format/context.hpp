#ifndef UUID_C2DF26B7_DE7D_47B8_BAEE_9F6EEBB12891
#define UUID_C2DF26B7_DE7D_47B8_BAEE_9F6EEBB12891
#pragma once

#include "item.hpp"
#include "../dumpable.hpp"

#include <boost/intrusive/set.hpp>
#include <string>
#include <map>

namespace Neptools
{

  class Context : public ItemWithChildren
  {
    LIBSHIT_LUA_CLASS;
  public:
    Context();
    ~Context();

    void Fixup() override;

    template <typename T, typename... Args>
    LIBSHIT_NOLUA Libshit::NotNull<Libshit::SmartPtr<T>> Create(Args&&... args)
    {
      return Libshit::MakeSmart<T>(
        Item::Key{}, *this, std::forward<Args>(args)...);
    }

    Libshit::NotNull<LabelPtr> GetLabel(const std::string& name) const;
    Libshit::NotNull<LabelPtr> CreateLabel(std::string name, ItemPointer ptr);
    Libshit::NotNull<LabelPtr> CreateLabelFallback(
        const std::string& name, ItemPointer ptr);
    Libshit::NotNull<LabelPtr> CreateLabelFallback(
        const std::string& name, FilePosition pos)
    { return CreateLabelFallback(name, GetPointer(pos)); }

    Libshit::NotNull<LabelPtr> CreateOrSetLabel(std::string name, ItemPointer ptr);
    Libshit::NotNull<LabelPtr> GetOrCreateDummyLabel(std::string name);

    Libshit::NotNull<LabelPtr> GetLabelTo(ItemPointer ptr);
    Libshit::NotNull<LabelPtr> GetLabelTo(FilePosition pos)
    { return GetLabelTo(GetPointer(pos)); }

    Libshit::NotNull<LabelPtr> GetLabelTo(FilePosition pos, std::string name);

    ItemPointer GetPointer(FilePosition pos) const noexcept;

    void Dispose() noexcept override;

  protected:
    void SetupParseFrom(Item& item);

  private:
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

  struct PrintLabelStruct { const Label* label; };
  std::ostream& operator<<(std::ostream& os, PrintLabelStruct label);
  inline PrintLabelStruct PrintLabel(const LabelPtr& label)
  { return {label.get()}; }
}

#endif
