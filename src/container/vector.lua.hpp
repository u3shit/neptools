#ifndef UUID_E374ABCE_6BEB_482C_9506_4E4AE9174D22
#define UUID_E374ABCE_6BEB_482C_9506_4E4AE9174D22
#pragma once

#include <vector>

#include "../lua/user_type.hpp"

namespace Neptools::Lua
{

// todo: other types
template <typename T> struct Nullable;
template <typename T, template <typename> class Storage>
struct Nullable<WeakPtrBase<T, Storage>>
{ using Type = WeakPtrBase<T, Storage>; };

template <typename T, typename Allocator>
struct IsSmartObject<std::vector<T, Allocator>> : std::true_type {};

template <typename T, typename Allocator = std::allocator<T>>
struct VectorBinding
{
    using Vect = std::vector<T, Allocator>;
    using size_type = typename Vect::size_type;

    static void Assign(Vect& v, const Vect& o) { v = o; }

    static typename Nullable<T>::Type Get(Vect& v, size_type i) noexcept
    {
        if (i < v.size()) return v[i];
        else return nullptr;
    }

    static void Set(Vect& v, size_type i, const T& val)
    {
        if (i >= v.size()) v.resize(i+1);
        v[i] = val;
    }

    static auto CheckedNthEnd(Vect& v, size_type i)
    {
        if (i <= v.size()) return v.begin() + i;
        else NEPTOOLS_THROW(std::out_of_range{"VectorBinding::CheckedNthEnd"});
    }

    static auto IndexOf(Vect& v, typename Vect::iterator it) noexcept
    { return it - v.begin(); }

    static auto Insert0(Vect& v, size_type pos, size_type count, const T& val)
    { return IndexOf(v, v.insert(CheckedNthEnd(v, pos), count, val)); }

    static auto Insert1(Vect& v, size_type pos, const T& val)
    { return IndexOf(v, v.insert(CheckedNthEnd(v, pos), val)); }


    static auto Erase0(StateRef vm, Vect& v, size_type s, size_type e)
    {
        if (s > e) luaL_error(vm, "Invalid range");
        return IndexOf(v, v.erase(CheckedNthEnd(v, s), CheckedNthEnd(v, e)));
    }

    static auto Erase1(Vect& v, size_type pos)
    { return IndexOf(v, v.erase(CheckedNthEnd(v, pos))); }

    // lua-compat: returns the erased element
    static T Remove(Vect& v, size_type i)
    {
        auto ret = v.at(i);
        v.erase(v.begin() + i);
        return ret;
    }

    static void PopBack(Vect& v) noexcept
    { if (!v.empty()) v.pop_back(); }

    static Lua::RetNum ToTable(Lua::StateRef vm, Vect& v)
    {
        auto size = v.size();
        lua_createtable(vm, size ? size-1 : size, 0);
        for (size_t i = 0; i < size; ++i)
        {
            vm.Push(v[i]);
            lua_rawseti(vm, -2, i);
        }
        return 1;
    }

    static void Register(TypeBuilder& bld)
    {
#define AP(...) decltype(__VA_ARGS__), __VA_ARGS__
        bld.Add<
            Overload<AP(MakeShared<Vect, size_type, const T&>)>,
            Overload<AP(MakeShared<Vect, size_type>)>,
            Overload<AP(MakeShared<Vect, const Vect&>)>,
            Overload<AP(MakeShared<Vect>)>
        >("new");

        bld.Add<
            Overload<void (Vect::*)(size_type, const T&), &Vect::assign>,
            Overload<AP(&Assign)>
        >("assign");

#define S(name) bld.Add<AP(&Vect::name)>(#name)
        S(empty); S(size); S(max_size); S(reserve); S(capacity);
        S(shrink_to_fit); S(clear); S(swap);
#undef S
        bld.Add<AP(&Get)>("get");
        bld.Add<AP(&Set)>("set");
        bld.Add<T& (Vect::*)(size_t), &Vect::at>("at");
        bld.Add<AP(&Vect::size)>("__len");
        bld.Add<Overload<AP(&Insert0)>, Overload<AP(&Insert1)>>("insert");
        bld.Add<Overload<AP(&Erase0)>, Overload<AP(&Erase1)>>("erase");
        bld.Add<AP(&Remove)>("remove");
        bld.Add<void (Vect::*)(const T&), &Vect::push_back>("push_back");
        bld.Add<AP(&PopBack)>("pop_back");
        bld.Add<
            Overload<void (Vect::*)(size_type, const T&), &Vect::resize>,
            Overload<void (Vect::*)(size_type), &Vect::resize>
        >("resize");
        bld.Add<AP(&ToTable)>("to_table");

        bld.Add<bool (*)(const Vect&, const Vect&), &std::operator==>("__eq");
        bld.Add<bool (*)(const Vect&, const Vect&), &std::operator<>("__lt");
        bld.Add<bool (*)(const Vect&, const Vect&), &std::operator<=>("__le");
#undef AP

        luaL_getmetatable(bld, "neptools_ipairs");
        bld.SetField("__ipairs");
    }
};

}

#define NEPTOOLS_STD_VECTOR_LUAGEN(name, ...)                               \
    template<> void Neptools::Lua::TypeRegister::DoRegister<                \
        ::std::vector<__VA_ARGS__>>(TypeBuilder& bld)                       \
    { ::Neptools::Lua::VectorBinding<__VA_ARGS__>::Register(bld); }         \
                                                                            \
    static ::Neptools::Lua::TypeRegister::StateRegister<                    \
        ::std::vector<__VA_ARGS__>> reg_std_vector_##name;                  \
    template<> struct ::Neptools::Lua::TypeName<std::vector<__VA_ARGS__>>   \
    { static constexpr const char* TYPE_NAME = "neptools.vector_" #name; }

#endif
