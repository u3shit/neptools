#ifndef UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#define UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#pragma once

#include <type_traits>
#include "../meta.hpp"
#include "../not_null.hpp"
#include "value_object.hpp"
#include "userdata.hpp"

namespace Neptools
{
namespace Lua
{

class NEPTOOLS_LUAGEN(no_inherit=true,smart_object=true) SmartObject {};

// specialize if needed
template <typename T, typename Enable = void>
struct IsSmartObject : std::is_base_of<SmartObject, T> {};

// smart objects are no longer value objects
template <typename T>
struct IsValueObject<T, std::enable_if_t<IsSmartObject<T>::value>>
    : std::false_type {};

template <typename T>
struct IsUserdataObject<T, std::enable_if_t<IsSmartObject<T>::value>>
    : std::true_type {};


template <typename T>
struct IsRefCountedSmartObject : std::integral_constant<
    bool, IsSmartObject<T>::value && std::is_base_of<RefCounted, T>::value> {};

template <typename T>
struct IsNormalSmartObject : std::integral_constant<
    bool, IsSmartObject<T>::value && !std::is_base_of<RefCounted, T>::value> {};

class NEPTOOLS_LUAGEN(no_inherit=true) DynamicObject : public SmartObject
{
public:
    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;

    virtual void PushLua(StateRef vm, RefCounted& ctrl) = 0;
};

template <typename T>
struct IsSelfPushableDynamicObject : std::integral_constant<
    bool,
    std::is_base_of<DynamicObject, T>::value &&
        std::is_base_of<RefCounted, T>::value> {};

#define NEPTOOLS_THIS_TYPE std::remove_pointer_t<decltype(this)>
#define NEPTOOLS_DYNAMIC_OBJ_GEN(...)                                        \
    NEPTOOLS_LUA_CLASS;                                                      \
    private:                                                                 \
    void PushLua(::Neptools::Lua::StateRef vm,                               \
                 ::Neptools::RefCounted& ctrl) override                      \
    {                                                                        \
        ::Neptools::Lua::UserdataDetail::Push<__VA_ARGS__>(                  \
            vm, ctrl, this, &::Neptools::Lua::TYPE_TAG<NEPTOOLS_THIS_TYPE>); \
    }

#define NEPTOOLS_DYNAMIC_OBJECT                                                 \
    NEPTOOLS_DYNAMIC_OBJ_GEN(                                                   \
        std::conditional_t<                                                     \
            std::is_base_of<::Neptools::RefCounted, NEPTOOLS_THIS_TYPE>::value, \
            ::Neptools::Lua::RefCountedUserdata, \
            ::Neptools::Lua::SharedUserdata>)

inline DynamicObject& GetDynamicObject(DynamicObject& obj) { return obj; }

namespace Detail
{
template <typename T, typename Enable = void>
struct SmartPush
{
    static void Push(StateRef& vm, RefCounted& ctrl, T& ptr)
    {
        using UD = std::conditional_t<
            std::is_base_of<RefCounted, T>::value,
            RefCountedUserdata,
            SharedUserdata>;
        UserdataDetail::Push<UD>(vm, ctrl, &ptr, &TYPE_TAG<T>);
    }
};

template <typename T>
struct SmartPush<T, std::enable_if_t<std::is_base_of<DynamicObject, T>::value>>
{
    static void Push(StateRef& vm, RefCounted& ctrl, T& obj)
    { GetDynamicObject(obj).PushLua(vm, ctrl); }
};
}

template <typename T>
struct TypeTraits<T, std::enable_if_t<IsNormalSmartObject<T>::value>>
    : UserdataTraits<T> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<IsSelfPushableDynamicObject<T>::value>>
    : UserdataTraits<T>
{
    static void Push(StateRef vm, T& obj)
    { GetDynamicObject(obj).PushLua(vm, obj); }
};

template <typename T, template<typename> class Storage>
struct TypeTraits<NotNull<SharedPtrBase<T, Storage>>,
                  std::enable_if_t<IsSmartObject<T>::value>>
    : UserdataTraits<SharedPtrBase<T, Storage>>
{
    static void Push(StateRef vm, const NotNull<SharedPtrBase<T, Storage>>& obj)
    { Detail::SmartPush<T>::Push(vm, *obj.Get().GetCtrl(), *obj); }
};

template <typename T, template<typename> class Storage>
struct TypeTraits<SharedPtrBase<T, Storage>,
                  std::enable_if_t<IsSmartObject<T>::value>>
    : NullableTypeTraits<SharedPtrBase<T, Storage>> {};

template <typename T, template<typename> class Storage>
struct TypeTraits<WeakPtrBase<T, Storage>,
                  std::enable_if_t<IsSmartObject<T>::value>>
{
    using Type = WeakPtrBase<T, Storage>;

    static Type Get(StateRef vm, bool arg, int idx)
    { return lua_isnil(vm, idx) ? nullptr : Type{&UserdataTraits<T>::Get(vm, arg, idx)}; }
    static Type UnsafeGet(StateRef vm, int idx)
    { return lua_isnil(vm, idx) ? nullptr : Type{&UserdataTraits<T>::UnsafeGet(vm, idx)}; }
    static bool Is(StateRef vm, int idx)
    { return lua_isnil(vm, idx) || UserdataTraits<T>::Is(vm, idx); }

    static void Push(StateRef vm, const Type& obj)
    {
        auto sptr = obj.lock();
        if (sptr) Detail::SmartPush<T>::Push(vm, *obj.Get().GetCtrl(), *obj);
        else lua_pushnil(vm);
    }
};

}
}

#endif
