// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Cl3::TYPE_NAME[] = "neptools.cl3";

const char ::Neptools::Cl3::Entry::TYPE_NAME[] = "neptools.cl3.entry";
template <>
const char ::cl3_entry::TYPE_NAME[] = "neptools.ordered_map_cl3_entry";

namespace Libshit::Lua
{

  // class neptools.cl3
  template<>
  void TypeRegisterTraits<::Neptools::Cl3>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Cl3, ::Neptools::Dumpable>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Cl3>::Make<LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Cl3>::Make<>,
      &::Libshit::Lua::TypeTraits<::Neptools::Cl3>::Make<LuaGetRef<::Libshit::Lua::StateRef>, LuaGetRef<::uint32_t>, LuaGetRef<::Libshit::Lua::RawTable>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>
    >("get_field_14");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>
    >("set_field_14");
    bld.AddFunction<
      &::Libshit::Lua::GetRefCountedOwnedMember<::Neptools::Cl3, ::Neptools::Cl3::Entries, &::Neptools::Cl3::entries>
    >("get_entries");
    bld.AddFunction<
      static_cast<::uint32_t (::Neptools::Cl3::*)(const Libshit::WeakSmartPtr<::Neptools::Cl3::Entry> &) const noexcept>(&::Neptools::Cl3::IndexOf)
    >("index_of");
    bld.AddFunction<
      static_cast<::Neptools::Cl3::Entry & (::Neptools::Cl3::*)(::Libshit::StringView)>(&::Neptools::Cl3::GetOrCreateFile)
    >("get_or_create_file");
    bld.AddFunction<
      static_cast<void (::Neptools::Cl3::*)(const ::boost::filesystem::path &) const>(&::Neptools::Cl3::ExtractTo)
    >("extract_to");
    bld.AddFunction<
      static_cast<void (::Neptools::Cl3::*)(const ::boost::filesystem::path &)>(&::Neptools::Cl3::UpdateFromDir)
    >("update_from_dir");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::File & (::Neptools::Cl3::*)()>(&::Neptools::Cl3::GetStcm)
    >("get_stcm");

  }
  static TypeRegister::StateRegister<::Neptools::Cl3> reg_neptools_cl3;

  // class neptools.cl3.entry
  template<>
  void TypeRegisterTraits<::Neptools::Cl3::Entry>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>
    >("get_name");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>
    >("set_name");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>
    >("get_field_200");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>
    >("set_field_200");
    bld.AddFunction<
      &::Libshit::Lua::GetRefCountedOwnedMember<::Neptools::Cl3::Entry, ::Neptools::Cl3::Entry::Links, &::Neptools::Cl3::Entry::links>
    >("get_links");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Cl3::Entry, Libshit::SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>
    >("get_src");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Cl3::Entry, Libshit::SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>
    >("set_src");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>, LuaGetRef<::uint32_t>, LuaGetRef<Libshit::SmartPtr<::Neptools::Dumpable>>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Cl3::Entry> reg_neptools_cl3_entry;

  // class neptools.ordered_map_cl3_entry
  template<>
  void TypeRegisterTraits<::cl3_entry>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::cl3_entry>::Make<>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Cl3::Entry & (::cl3_entry::*)(::size_t)>(&::cl3_entry::at)
    >("at");
    bld.AddFunction<
      static_cast<::Neptools::Cl3::Entry & (::cl3_entry::*)()>(&::cl3_entry::front<Check::Throw>)
    >("front");
    bld.AddFunction<
      static_cast<::Neptools::Cl3::Entry & (::cl3_entry::*)()>(&::cl3_entry::back<Check::Throw>)
    >("back");
    bld.AddFunction<
      static_cast<bool (::cl3_entry::*)() const noexcept>(&::cl3_entry::empty)
    >("empty");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)() const noexcept>(&::cl3_entry::size)
    >("__len");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)() const noexcept>(&::cl3_entry::size)
    >("size");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)() const noexcept>(&::cl3_entry::max_size)
    >("max_size");
    bld.AddFunction<
      static_cast<void (::cl3_entry::*)(::size_t)>(&::cl3_entry::reserve)
    >("reserve");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)() const noexcept>(&::cl3_entry::capacity)
    >("capacity");
    bld.AddFunction<
      static_cast<void (::cl3_entry::*)()>(&::cl3_entry::shrink_to_fit)
    >("shrink_to_fit");
    bld.AddFunction<
      static_cast<void (::cl3_entry::*)() noexcept>(&::cl3_entry::clear)
    >("clear");
    bld.AddFunction<
      static_cast<void (::cl3_entry::*)() noexcept>(&::cl3_entry::pop_back<Check::Throw>)
    >("pop_back");
    bld.AddFunction<
      static_cast<void (::cl3_entry::*)(::cl3_entry &)>(&::cl3_entry::swap)
    >("swap");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)(const ::Neptools::Cl3::Entry &) const>(&::cl3_entry::index_of<Check::Throw>)
    >("index_of");
    bld.AddFunction<
      static_cast<::size_t (::cl3_entry::*)(const ::cl3_entry::key_type &) const>(&::cl3_entry::count)
    >("count");
    bld.AddFunction<
      static_cast<Libshit::SmartPtr<::Neptools::Cl3::Entry> (*)(::cl3_entry &, ::size_t) noexcept>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get),
      static_cast<Libshit::SmartPtr<::Neptools::Cl3::Entry> (*)(::cl3_entry &, const typename ::cl3_entry::key_type &)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get),
      static_cast<void (*)(::cl3_entry &, ::Libshit::Lua::Skip) noexcept>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get)
    >("get");
    bld.AddFunction<
      static_cast<std::tuple<bool, ::size_t> (*)(::cl3_entry &, ::size_t, ::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::NotNullPtr &&)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::insert)
    >("insert");
    bld.AddFunction<
      static_cast<::size_t (*)(::cl3_entry &, ::size_t, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::erase),
      static_cast<::size_t (*)(::cl3_entry &, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::erase)
    >("erase");
    bld.AddFunction<
      static_cast<::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::NotNullPtr (*)(::cl3_entry &, ::size_t)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::remove)
    >("remove");
    bld.AddFunction<
      static_cast<std::tuple<bool, ::size_t> (*)(::cl3_entry &, ::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::NotNullPtr &&)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::push_back)
    >("push_back");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::cl3_entry &, const typename ::cl3_entry::key_type &)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::find)
    >("find");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::cl3_entry &)>(::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::to_table)
    >("to_table");
    luaL_getmetatable(bld, "neptools_ipairs");    bld.SetField("__ipairs");
  }
  static TypeRegister::StateRegister<::cl3_entry> reg_neptools_ordered_map_cl3_entry;

}
#endif
