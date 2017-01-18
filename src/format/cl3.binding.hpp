// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.cl3
template<>
void TypeRegister::DoRegister<Neptools::Cl3>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;
    bld.Inherit<Neptools::Cl3, Neptools::Dumpable>();

    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<Neptools::Cl3>::Make<LuaGetRef<Neptools::Source>>), &::Neptools::Lua::TypeTraits<Neptools::Cl3>::Make<LuaGetRef<Neptools::Source>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<Neptools::Cl3>::Make<>), &::Neptools::Lua::TypeTraits<Neptools::Cl3>::Make<>>
    >("new");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3, uint32_t, &Neptools::Cl3::field_14>), &::Neptools::Lua::GetMember<Neptools::Cl3, uint32_t, &Neptools::Cl3::field_14>
    >("get_field_14");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Neptools::Cl3, uint32_t, &Neptools::Cl3::field_14>), &::Neptools::Lua::SetMember<Neptools::Cl3, uint32_t, &Neptools::Cl3::field_14>
    >("set_field_14");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3, OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>, &Neptools::Cl3::entries>), &::Neptools::Lua::GetMember<Neptools::Cl3, OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>, &Neptools::Cl3::entries>
    >("get_entries");
    bld.Add<
        uint32_t (Neptools::Cl3::*)(const WeakSmartPtr<Neptools::Cl3::Entry> &) const, &Neptools::Cl3::IndexOf
    >("index_of");
    bld.Add<
        Neptools::Cl3::Entry & (Neptools::Cl3::*)(StringView), &Neptools::Cl3::GetOrCreateFile
    >("get_or_create_file");
    bld.Add<
        void (Neptools::Cl3::*)(const boost::filesystem::path &) const, &Neptools::Cl3::ExtractTo
    >("extract_to");
    bld.Add<
        void (Neptools::Cl3::*)(const boost::filesystem::path &), &Neptools::Cl3::UpdateFromDir
    >("update_from_dir");
    bld.Add<
        Stcm::File & (Neptools::Cl3::*)(), &Neptools::Cl3::GetStcm
    >("get_stcm");

}
static TypeRegister::StateRegister<Neptools::Cl3> reg_neptools_cl3;

}
}


const char Neptools::Cl3::TYPE_NAME[] = "neptools.cl3";

namespace Neptools
{
namespace Lua
{

// class neptools.cl3.entry
template<>
void TypeRegister::DoRegister<Neptools::Cl3::Entry>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3::Entry, std::string, &Neptools::Cl3::Entry::name>), &::Neptools::Lua::GetMember<Neptools::Cl3::Entry, std::string, &Neptools::Cl3::Entry::name>
    >("get_name");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Neptools::Cl3::Entry, std::string, &Neptools::Cl3::Entry::name>), &::Neptools::Lua::SetMember<Neptools::Cl3::Entry, std::string, &Neptools::Cl3::Entry::name>
    >("set_name");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3::Entry, uint32_t, &Neptools::Cl3::Entry::field_200>), &::Neptools::Lua::GetMember<Neptools::Cl3::Entry, uint32_t, &Neptools::Cl3::Entry::field_200>
    >("get_field_200");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Neptools::Cl3::Entry, uint32_t, &Neptools::Cl3::Entry::field_200>), &::Neptools::Lua::SetMember<Neptools::Cl3::Entry, uint32_t, &Neptools::Cl3::Entry::field_200>
    >("set_field_200");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3::Entry, std::vector<WeakRefCountedPtr<Cl3::Entry> >, &Neptools::Cl3::Entry::links>), &::Neptools::Lua::GetMember<Neptools::Cl3::Entry, std::vector<WeakRefCountedPtr<Cl3::Entry> >, &Neptools::Cl3::Entry::links>
    >("get_links");
    bld.Add<
        decltype(&::Neptools::Lua::GetMember<Neptools::Cl3::Entry, SmartPtr<Neptools::Dumpable>, &Neptools::Cl3::Entry::src>), &::Neptools::Lua::GetMember<Neptools::Cl3::Entry, SmartPtr<Neptools::Dumpable>, &Neptools::Cl3::Entry::src>
    >("get_src");
    bld.Add<
        decltype(&::Neptools::Lua::SetMember<Neptools::Cl3::Entry, SmartPtr<Neptools::Dumpable>, &Neptools::Cl3::Entry::src>), &::Neptools::Lua::SetMember<Neptools::Cl3::Entry, SmartPtr<Neptools::Dumpable>, &Neptools::Cl3::Entry::src>
    >("set_src");
    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>, LuaGetRef<uint32_t>, LuaGetRef<SmartPtr<Neptools::Dumpable>>>), &::Neptools::Lua::TypeTraits<Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>, LuaGetRef<uint32_t>, LuaGetRef<SmartPtr<Neptools::Dumpable>>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>>), &::Neptools::Lua::TypeTraits<Neptools::Cl3::Entry>::Make<LuaGetRef<std::string>>>
    >("new");

}
static TypeRegister::StateRegister<Neptools::Cl3::Entry> reg_neptools_cl3_entry;

}
}


const char Neptools::Cl3::Entry::TYPE_NAME[] = "neptools.cl3.entry";

namespace Neptools
{
namespace Lua
{

// class neptools.ordered_map_cl3_entry
template<>
void TypeRegister::DoRegister<cl3_entry>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<cl3_entry>::Make<>), &::Neptools::Lua::TypeTraits<cl3_entry>::Make<>
    >("new");
    bld.Add<
        Neptools::Cl3::Entry & (cl3_entry::*)(size_t), &cl3_entry::at
    >("at");
    bld.Add<
        bool (cl3_entry::*)() const, &cl3_entry::empty
    >("empty");
    bld.Add<
        size_t (cl3_entry::*)() const, &cl3_entry::size
    >("__len");
    bld.Add<
        size_t (cl3_entry::*)() const, &cl3_entry::size
    >("size");
    bld.Add<
        size_t (cl3_entry::*)() const, &cl3_entry::max_size
    >("max_size");
    bld.Add<
        void (cl3_entry::*)(size_t), &cl3_entry::reserve
    >("reserve");
    bld.Add<
        size_t (cl3_entry::*)() const, &cl3_entry::capacity
    >("capacity");
    bld.Add<
        void (cl3_entry::*)(), &cl3_entry::shrink_to_fit
    >("shrink_to_fit");
    bld.Add<
        void (cl3_entry::*)(), &cl3_entry::clear
    >("clear");
    bld.Add<
        void (cl3_entry::*)(), &cl3_entry::checked_pop_back
    >("pop_back");
    bld.Add<
        void (cl3_entry::*)(cl3_entry &), &cl3_entry::swap
    >("swap");
    bld.Add<
        size_t (cl3_entry::*)(const cl3_entry::key_type&) const, &cl3_entry::count
    >("count");
    bld.Add<
        Overload<SmartPtr<Neptools::Cl3::Entry> (*)(OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, size_t), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::get>,
        Overload<SmartPtr<Neptools::Cl3::Entry> (*)(cl3_entry&, const typename cl3_entry::key_type&), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::get>
    >("get");
    bld.Add<
        std::tuple<bool, size_t> (*)(OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, size_t, const NotNull<SmartPtr<Neptools::Cl3::Entry> > &), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::insert
    >("insert");
    bld.Add<
        Overload<size_t (*)(Lua::StateRef, OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, size_t, size_t), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::erase>,
        Overload<size_t (*)(OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, size_t), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::erase>
    >("erase");
    bld.Add<
        NotNull<SmartPtr<Neptools::Cl3::Entry> > (*)(OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, size_t), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::remove
    >("remove");
    bld.Add<
        std::tuple<bool, size_t> (*)(OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &, const NotNull<SmartPtr<Neptools::Cl3::Entry> > &), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::push_back
    >("push_back");
    bld.Add<
        Lua::RetNum (*)(StateRef, cl3_entry&, const typename cl3_entry::key_type&), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::find
    >("find");
    bld.Add<
        Lua::RetNum (*)(Lua::StateRef, OrderedMap<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue, std::less<std::basic_string<char> > > &), &Neptools::OrderedMapLua<Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue>::to_table
    >("to_table");
  luaL_getmetatable(vm, "neptools_ipairs");  bld.SetField("__ipairs");
}
static TypeRegister::StateRegister<cl3_entry> reg_neptools_ordered_map_cl3_entry;

}
}

template <>
const char cl3_entry::TYPE_NAME[] = "neptools.ordered_map_cl3_entry";

