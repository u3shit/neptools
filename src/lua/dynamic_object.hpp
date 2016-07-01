#ifndef UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#define UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#pragma once

#include <type_traits>
#include "../not_null.hpp"
#include "userdata.hpp"

namespace Neptools
{
namespace Lua
{

class DynamicObject
{
public:
    static constexpr const char* TYPE_NAME = "neptools.object";

    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;

private:
    template <typename T, typename> friend struct TypeTraits;
    void PushLua(StateRef vm, RefCounted& ctrl);
    // push lua userdata, without caching
    virtual void PushLuaObj(StateRef vm, RefCounted& ctrl) = 0;
};

template <typename Base, typename... Deriv> struct IsBaseOfAny;
template <typename Base> struct IsBaseOfAny<Base> : std::false_type {};
template <typename Base, typename Deriv, typename... Rest>
struct IsBaseOfAny<Base, Deriv, Rest...>
    : std::integral_constant<
        bool,
        std::is_base_of<Base, Deriv>::value ||
            IsBaseOfAny<Base, Rest...>::value>
{};


template <typename Userdata, typename T>
BOOST_FORCEINLINE
void PushLuaObjImpl(T* thiz, StateRef vm, RefCounted& ctrl)
{
    auto ud = lua_newuserdata(vm, sizeof(Userdata)); // +1
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &TYPE_TAG<T>); // +2
    NEPTOOLS_ASSERT(type == LUA_TTABLE);

    new (ud) Userdata{&ctrl, thiz};
    lua_setmetatable(vm, -2); // +1
}

#define NEPTOOLS_DYNAMIC_OBJ_GEN(...)                                           \
    private:                                                                    \
    void PushLuaObj(::Neptools::Lua::StateRef vm, ::Neptools::RefCounted& ctrl) override \
    { ::Neptools::Lua::PushLuaObjImpl<__VA_ARGS__>(this, vm, ctrl); }

#define NEPTOOLS_DYNAMIC_OBJECT                                                 \
    NEPTOOLS_DYNAMIC_OBJ_GEN(                                                   \
        std::conditional_t<                                                     \
            std::is_base_of<::Neptools::RefCounted, std::remove_pointer_t<decltype(this)>>::value, \
            ::Neptools::Lua::RefCountedUserdata<std::remove_pointer_t<decltype(this)>>, \
            ::Neptools::Lua::SharedUserdata>)


template <typename T>
struct TypeTraits<T, std::enable_if_t<
                         std::is_base_of<DynamicObject, T>::value &&
                         !std::is_base_of<RefCounted, T>::value>>
    : UserdataTraits<T> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<
                         std::is_base_of<DynamicObject, T>::value &&
                         std::is_base_of<RefCounted, T>::value>>
    : UserdataTraits<T>
{
    static void Push(StateRef vm, T& obj)
    { static_cast<DynamicObject&>(obj).PushLua(vm, &obj); }
};


template <typename T>
struct TypeTraits<
    NotNull<SharedPtr<T>>,
    std::enable_if_t<
        std::is_base_of<DynamicObject, T>::value &&
        !std::is_base_of<RefCounted, T>::value>>
{
    static void Push(StateRef vm, const NotNull<SharedPtr<T>>& ptr)
    { static_cast<DynamicObject&>(*ptr).PushLua(vm, *ptr.Get().GetCtrl()); }
};

}
}

#endif
