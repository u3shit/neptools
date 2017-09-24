// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Label::TYPE_NAME[] = "neptools.label";

const char ::Neptools::Item::TYPE_NAME[] = "neptools.item";

const char ::Neptools::ItemWithChildren::TYPE_NAME[] = "neptools.item_with_children";
template <>
const char ::item::TYPE_NAME[] = "neptools.parent_list_item";

namespace Libshit::Lua
{

  // class neptools.label
  template<>
  void TypeRegisterTraits<::Neptools::Label>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Label>::Make<LuaGetRef<std::string>, LuaGetRef<::Neptools::ItemPointer>>
    >("new");
    bld.AddFunction<
      static_cast<const std::string & (::Neptools::Label::*)() const>(&::Neptools::Label::GetName)
    >("get_name");
    bld.AddFunction<
      static_cast<const ::Neptools::ItemPointer & (::Neptools::Label::*)() const>(&::Neptools::Label::GetPtr)
    >("get_ptr");

  }
  static TypeRegister::StateRegister<::Neptools::Label> reg_neptools_label;

  // class neptools.item
  template<>
  void TypeRegisterTraits<::Neptools::Item>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Item, ::Neptools::Dumpable>();

    bld.AddFunction<
      static_cast<Libshit::RefCountedPtr<::Neptools::Context> (::Neptools::Item::*)() noexcept>(&::Neptools::Item::GetContextMaybe)
    >("get_context_maybe");
    bld.AddFunction<
      static_cast<::Libshit::NotNull<Libshit::RefCountedPtr<::Neptools::Context> > (::Neptools::Item::*)()>(&::Neptools::Item::GetContext)
    >("get_context");
    bld.AddFunction<
      static_cast<::Neptools::ItemWithChildren * (::Neptools::Item::*)() noexcept>(&::Neptools::Item::GetParent)
    >("get_parent");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Item::*)() const noexcept>(&::Neptools::Item::GetPosition)
    >("get_position");
    bld.AddFunction<
      static_cast<void (::Neptools::Item::*)(const ::Libshit::NotNull<Libshit::RefCountedPtr<::Neptools::Item> > &)>(&::Neptools::Item::Replace<Check::Throw>)
    >("replace");
    bld.AddFunction<
      TableRetWrap<static_cast<const ::Neptools::Item::LabelsContainer & (::Neptools::Item::*)() const>(&::Neptools::Item::GetLabels)>::Wrap
    >("get_labels");

  }
  static TypeRegister::StateRegister<::Neptools::Item> reg_neptools_item;

  // class neptools.item_with_children
  template<>
  void TypeRegisterTraits<::Neptools::ItemWithChildren>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::ItemWithChildren, ::Neptools::Item>();

    bld.AddFunction<
      OwnedSharedPtrWrap<static_cast<::Neptools::ItemList & (::Neptools::ItemWithChildren::*)() noexcept>(&::Neptools::ItemWithChildren::GetChildren)>::Wrap
    >("get_children");
 do { auto runbc_ret = luaL_loadbuffer( bld, luaJIT_BC_builder, luaJIT_BC_builder_SIZE, "builder"); (__builtin_expect(!!(runbc_ret == 0), 1) ? ((void)0) : ::Libshit::AssertFailed("runbc_ret == 0", nullptr, ([]{ struct X { static constexpr char Get(size_t i) { return ("src/format/item.hpp")[i]; } }; return ::Libshit::ToCharPackV< X, ::Libshit::FileTools::FileName, std::make_index_sequence<sizeof("src/format/item.hpp")-1>>{}; }()).str, 168, __PRETTY_FUNCTION__)); (void) runbc_ret; lua_call(bld, 0, 1); } while (false); bld.SetField("build"); 
  }
  static TypeRegister::StateRegister<::Neptools::ItemWithChildren> reg_neptools_item_with_children;

  // class neptools.parent_list_item
  template<>
  void TypeRegisterTraits<::item>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::item>::Make<>
    >("new");
    bld.AddFunction<
      static_cast<void (::item::*)(::item &) noexcept>(&::item::swap)
    >("swap");
    bld.AddFunction<
      static_cast<void (::item::*)(::item::reference)>(&::item::push_back<Check::Throw>)
    >("push_back");
    bld.AddFunction<
      static_cast<void (::item::*)()>(&::item::pop_back<Check::Throw>)
    >("pop_back");
    bld.AddFunction<
      static_cast<void (::item::*)(::item::reference)>(&::item::push_front<Check::Throw>)
    >("push_front");
    bld.AddFunction<
      static_cast<void (::item::*)()>(&::item::pop_front<Check::Throw>)
    >("pop_front");
    bld.AddFunction<
      static_cast<::item::reference (::item::*)()>(&::item::front<Check::Throw>)
    >("front");
    bld.AddFunction<
      static_cast<::item::reference (::item::*)()>(&::item::back<Check::Throw>)
    >("back");
    bld.AddFunction<
      static_cast<::item::size_type (::item::*)() const noexcept>(&::item::size)
    >("size");
    bld.AddFunction<
      static_cast<bool (::item::*)() const noexcept>(&::item::empty)
    >("empty");
    bld.AddFunction<
      static_cast<void (::item::*)(::item::size_type) noexcept>(&::item::shift_backwards)
    >("shift_backwards");
    bld.AddFunction<
      static_cast<void (::item::*)(::item::size_type) noexcept>(&::item::shift_forward)
    >("shift_forward");
    bld.AddFunction<
      static_cast<::item::iterator (::item::*)(::item::const_iterator)>(&::item::erase<Check::Throw>),
      static_cast<::item::iterator (::item::*)(::item::const_iterator, ::item::const_iterator)>(&::item::erase<Check::Throw>)
    >("erase");
    bld.AddFunction<
      static_cast<void (::item::*)() noexcept>(&::item::clear)
    >("clear");
    bld.AddFunction<
      static_cast<::item::iterator (::item::*)(::item::const_iterator, ::item::reference)>(&::item::insert<Check::Throw>)
    >("insert");
    bld.AddFunction<
      static_cast<void (::item::*)(::item::const_iterator, ::item &)>(&::item::splice<Check::Throw>),
      static_cast<void (::item::*)(::item::const_iterator, ::item &, ::item::const_iterator)>(&::item::splice<Check::Throw>),
      static_cast<void (::item::*)(::item::const_iterator, ::item &, ::item::const_iterator, ::item::const_iterator)>(&::item::splice<Check::Throw>)
    >("splice");
    bld.AddFunction<
      static_cast<void (::item::*)(::Libshit::Lua::FunctionWrapGen<bool>)>(&::item::sort<::Libshit::Lua::FunctionWrapGen<bool>>)
    >("sort");
    bld.AddFunction<
      static_cast<void (::item::*)(::item &, ::Libshit::Lua::FunctionWrapGen<bool>)>(&::item::merge<::Libshit::Check::Throw, ::Libshit::Lua::FunctionWrapGen<bool>>)
    >("merge");
    bld.AddFunction<
      static_cast<void (::item::*)() noexcept>(&::item::reverse)
    >("reverse");
    bld.AddFunction<
      static_cast<void (::item::*)(::Libshit::Lua::FunctionWrapGen<bool>)>(&::item::remove_if<::Libshit::Lua::FunctionWrapGen<bool>>)
    >("remove_if");
    bld.AddFunction<
      static_cast<void (::item::*)(::Libshit::Lua::FunctionWrapGen<bool>)>(&::item::unique<::Libshit::Lua::FunctionWrapGen<bool>>)
    >("unique");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::FakeClass &, ::Neptools::Item &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::Next)
    >("next");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::FakeClass &, ::Neptools::Item &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::Prev)
    >("prev");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits, ::Neptools::ParentListBaseHookTraits<::Neptools::Item, ::Neptools::DefaultTag> >::FakeClass &)>(::Neptools::ParentListLua<::Neptools::Item, ::Neptools::ItemListTraits>::ToTable)
    >("to_table");

  }
  static TypeRegister::StateRegister<::item> reg_neptools_parent_list_item;

}
#endif
