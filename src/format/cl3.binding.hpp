// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.cl3
template<>
void TypeRegister::DoRegister<::Neptools::Cl3>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Cl3, ::Neptools::Dumpable>();

    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Cl3>::Make<LuaGetRef<::Neptools::Source>>), &::Neptools::Lua::TypeTraits<::Neptools::Cl3>::Make<LuaGetRef<::Neptools::Source>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Cl3>::Make<>), &::Neptools::Lua::TypeTraits<::Neptools::Cl3>::Make<>>
    >("new");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>), &::Neptools::Lua::GetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>
    >("get_field_14");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>), &::Neptools::Lua::SetMember<::Neptools::Cl3, ::uint32_t, &::Neptools::Cl3::field_14>
    >("set_field_14");
    bld.Add<
        decltype(&::Neptools::Lua::GetSmartOwnedMember<::Neptools::Cl3, ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>, &::Neptools::Cl3::entries>), &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Cl3, ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>, &::Neptools::Cl3::entries>
    >("get_entries");
    bld.Add<
        ::uint32_t (::Neptools::Cl3::*)(const WeakSmartPtr<::Neptools::Cl3::Entry> &) const, &::Neptools::Cl3::IndexOf
    >("index_of");
    bld.Add<
        ::Neptools::Cl3::Entry & (::Neptools::Cl3::*)(::Neptools::StringView), &::Neptools::Cl3::GetOrCreateFile
    >("get_or_create_file");
    bld.Add<
        void (::Neptools::Cl3::*)(const ::boost::filesystem::path &) const, &::Neptools::Cl3::ExtractTo
    >("extract_to");
    bld.Add<
        void (::Neptools::Cl3::*)(const ::boost::filesystem::path &), &::Neptools::Cl3::UpdateFromDir
    >("update_from_dir");
    bld.Add<
        ::Neptools::Stcm::File & (::Neptools::Cl3::*)(), &::Neptools::Cl3::GetStcm
    >("get_stcm");

}
static TypeRegister::StateRegister<::Neptools::Cl3> reg_neptools_cl3;

}
}


const char ::Neptools::Cl3::TYPE_NAME[] = "neptools.cl3";

namespace Neptools
{
namespace Lua
{

// class neptools.cl3.entry
template<>
void TypeRegister::DoRegister<::Neptools::Cl3::Entry>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>), &::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>
    >("get_name");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>), &::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, std::string, &::Neptools::Cl3::Entry::name>
    >("set_name");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>), &::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>
    >("get_field_200");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>), &::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, ::uint32_t, &::Neptools::Cl3::Entry::field_200>
    >("set_field_200");
    bld.Add<
        decltype(&::Neptools::Lua::GetSmartOwnedMember<::Neptools::Cl3::Entry, std::vector<WeakRefCountedPtr<::Neptools::Cl3::Entry> >, &::Neptools::Cl3::Entry::links>), &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Cl3::Entry, std::vector<WeakRefCountedPtr<::Neptools::Cl3::Entry> >, &::Neptools::Cl3::Entry::links>
    >("get_links");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>), &::Neptools::Lua::GetMember<::Neptools::Cl3::Entry, SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>
    >("get_src");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>), &::Neptools::Lua::SetMember<::Neptools::Cl3::Entry, SmartPtr<::Neptools::Dumpable>, &::Neptools::Cl3::Entry::src>
    >("set_src");
    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>, LuaGetRef<::uint32_t>, LuaGetRef<SmartPtr<::Neptools::Dumpable>>>), &::Neptools::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>, LuaGetRef<::uint32_t>, LuaGetRef<SmartPtr<::Neptools::Dumpable>>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>>), &::Neptools::Lua::TypeTraits<::Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Cl3::Entry> reg_neptools_cl3_entry;

}
}


const char ::Neptools::Cl3::Entry::TYPE_NAME[] = "neptools.cl3.entry";

namespace Neptools
{
namespace Lua
{

// class neptools.ordered_map_cl3_entry
template<>
void TypeRegister::DoRegister<::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >>::Make<>), &::Neptools::Lua::TypeTraits<::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >>::Make<>
    >("new");
    bld.Add<
        Neptools::Cl3::Entry & (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(::size_t), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::at
    >("at");
    bld.Add<
        bool (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::empty
    >("empty");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::size
    >("__len");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::size
    >("size");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::max_size
    >("max_size");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(::size_t), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::reserve
    >("reserve");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)() const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::capacity
    >("capacity");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::shrink_to_fit
    >("shrink_to_fit");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::clear
    >("clear");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::checked_pop_back
    >("pop_back");
    bld.Add<
        void (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &), &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::swap
    >("swap");
    bld.Add<
        ::size_t (::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::*)(const ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::key_type &) const, &::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::count
    >("count");
    bld.Add<
        Overload<SmartPtr<::Neptools::Cl3::Entry> (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get>,
        Overload<SmartPtr<::Neptools::Cl3::Entry> (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, const typename ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::key_type &), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get>,
        Overload<void (*)(), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::get>
    >("get");
    bld.Add<
        std::tuple<bool, size_t> (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::size_t, ::Neptools::NotNull<SmartPtr<::Neptools::Cl3::Entry> > &&), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::insert
    >("insert");
    bld.Add<
        Overload<::size_t (*)(::Neptools::Lua::StateRef, ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::size_t, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::erase>,
        Overload<::size_t (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::erase>
    >("erase");
    bld.Add<
        ::Neptools::NotNull<SmartPtr<::Neptools::Cl3::Entry> > (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::size_t), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::remove
    >("remove");
    bld.Add<
        std::tuple<bool, size_t> (*)(::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, ::Neptools::NotNull<SmartPtr<::Neptools::Cl3::Entry> > &&), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::push_back
    >("push_back");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &, const typename ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::key_type &), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::find
    >("find");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > > &), &::Neptools::OrderedMapLua<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue>::to_table
    >("to_table");
  luaL_getmetatable(bld, "neptools_ipairs");  bld.SetField("__ipairs");
}
static TypeRegister::StateRegister<::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >> reg_neptools_ordered_map_cl3_entry;

}
}

template <>
const char ::Neptools::OrderedMap<::Neptools::Cl3::Entry, ::Neptools::Cl3::EntryKeyOfValue, ::std::less<::std::basic_string<char> > >::TYPE_NAME[] = "neptools.ordered_map_cl3_entry";

