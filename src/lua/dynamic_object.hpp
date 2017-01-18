#ifndef UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#define UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#pragma once

#include <type_traits>
#include "../meta.hpp"
#include "../not_null.hpp"
#include "userdata.hpp"

// todo: move to external header or just wait for gcc 7
#if defined(__GLIBCXX__) && __cpp_lib_type_trait_variable_templates < 201510
#include <experimental/type_traits>
namespace std { using namespace experimental::fundamentals_v1; }
#endif

namespace Neptools
{
namespace Lua
{

class NEPTOOLS_LUAGEN(no_inherit=true) SmartObject {};

// specialize if needed
template <typename T, typename Enable = void>
struct IsSmartObject : std::is_base_of<SmartObject, T> {};

template <typename T>
constexpr bool is_smart_object_v = IsSmartObject<T>::value;

template <typename T>
struct IsUserdataObject<T, std::enable_if_t<IsSmartObject<T>::value>>
    : std::true_type {};


template <typename T>
constexpr bool is_ref_counted_smart_object_v =
    is_smart_object_v<T> && std::is_base_of_v<RefCounted, T>;

template <typename T>
constexpr bool is_normal_smart_object_v =
    is_smart_object_v<T> && !std::is_base_of_v<RefCounted, T>;

class NEPTOOLS_LUAGEN(no_inherit=true,smart_object=true) DynamicObject
    : public SmartObject
{
public:
    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;

    virtual void PushLua(StateRef vm, RefCounted& ctrl) = 0;
};

template <typename T>
constexpr bool is_self_pushable_dynamic_object_v =
    std::is_base_of_v<DynamicObject, T> && std::is_base_of_v<RefCounted, T>;

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
struct TypeTraits<T, std::enable_if_t<
        is_smart_object_v<T> && !is_self_pushable_dynamic_object_v<T>>>
    : UserdataTraits<T>, MakeSharedHelper<T, SmartPtr<T>> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<is_self_pushable_dynamic_object_v<T>>>
    : UserdataTraits<T>, MakeSharedHelper<T, SmartPtr<T>>
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
        if (sptr) Detail::SmartPush<T>::Push(vm, *sptr.GetCtrl(), *sptr);
        else lua_pushnil(vm);
    }
};

}
}

#endif
