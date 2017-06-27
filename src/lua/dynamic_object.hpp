#ifndef UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#define UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#pragma once

#ifdef NEPTOOLS_WITHOUT_LUA

namespace Neptools::Lua
{

struct SmartObject {};
class DynamicObject
{
public:
    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;
};

#define NEPTOOLS_DYNAMIC_OBJ_GEN(...) private: static void dummy_function()
#define NEPTOOLS_DYNAMIC_OBJECT NEPTOOLS_DYNAMIC_OBJ_GEN()

template <typename T, typename Enable = void> struct SmartObjectMaker;

}

#else

#include "ref_counted_user_data.hpp"
#include "../meta.hpp"
#include "../not_null.hpp"

#include <type_traits>

// todo: move to external header or just wait for gcc 7
#if defined(__GLIBCXX__) && __cpp_lib_type_trait_variable_templates < 201510
#include <experimental/type_traits>
namespace std { using namespace experimental::fundamentals_v1; }
#endif

namespace Neptools::Lua
{

class NEPTOOLS_LUAGEN(no_inherit=true) SmartObject {};

// specialize if needed
template <typename T, typename Enable = void>
struct IsSmartObject : std::is_base_of<SmartObject, T> {};

template <typename T>
constexpr bool IS_SMART_OBJECT = IsSmartObject<T>::value;

template <typename T>
struct IsUserDataObject<T, std::enable_if_t<IsSmartObject<T>::value>>
    : std::true_type {};


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
constexpr bool IS_SELF_PUSHABLE_DYNAMIC_OBJECT =
    std::is_base_of_v<DynamicObject, T> && std::is_base_of_v<RefCounted, T>;

#define NEPTOOLS_THIS_TYPE std::remove_pointer_t<decltype(this)>
#define NEPTOOLS_DYNAMIC_OBJ_GEN(...)                                        \
    private:                                                                 \
    void PushLua(::Neptools::Lua::StateRef vm,                               \
                 ::Neptools::RefCounted& ctrl) override                      \
    {                                                                        \
        ::Neptools::Lua::UserDataDetail::CreateCachedUserData<__VA_ARGS__>(  \
            vm, this, ::Neptools::Lua::TYPE_NAME<NEPTOOLS_THIS_TYPE>, &ctrl, this); \
    }

#define NEPTOOLS_DYNAMIC_OBJECT                                                 \
    NEPTOOLS_LUA_CLASS;                                                         \
    NEPTOOLS_DYNAMIC_OBJ_GEN(                                                   \
        std::conditional_t<                                                     \
            std::is_base_of<::Neptools::RefCounted, NEPTOOLS_THIS_TYPE>::value, \
            ::Neptools::Lua::RefCountedUserData, \
            ::Neptools::Lua::SharedUserData>)

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
            RefCountedUserData,
            SharedUserData>;
        UserDataDetail::CreateCachedUserData<UD>(
            vm, &ptr, TYPE_NAME<T>, &ctrl, &ptr);
    }
};

template <typename T>
struct SmartPush<T, std::enable_if_t<std::is_base_of<DynamicObject, T>::value>>
{
    static void Push(StateRef& vm, RefCounted& ctrl, T& obj)
    { GetDynamicObject(obj).PushLua(vm, ctrl); }
};
}

template <typename T, typename Enable = void>
struct SmartObjectMaker : MakeSharedHelper<T, SmartPtr<T>> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<
        IS_SMART_OBJECT<T> && !IS_SELF_PUSHABLE_DYNAMIC_OBJECT<T>>>
    : UserDataTraits<T>, SmartObjectMaker<T> {};

template <typename T>
struct TypeTraits<T, std::enable_if_t<IS_SELF_PUSHABLE_DYNAMIC_OBJECT<T>>>
    : UserDataTraits<T>, SmartObjectMaker<T>
{
    static void Push(StateRef vm, T& obj)
    { GetDynamicObject(obj).PushLua(vm, obj); }
};

template <typename T, template<typename> class Storage>
struct TypeTraits<NotNull<SharedPtrBase<T, Storage>>,
                  std::enable_if_t<IsSmartObject<T>::value>>
    : UserDataTraits<SharedPtrBase<T, Storage>>
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
    using RawType = T;

    template <bool Unsafe>
    static Type Get(StateRef vm, bool arg, int idx)
    {
        return lua_isnil(vm, idx) ? nullptr :
            Type{&UserDataTraits<T>::template Get<Unsafe>(vm, arg, idx)};
    }

    static bool Is(StateRef vm, int idx)
    { return lua_isnil(vm, idx) || UserDataTraits<T>::Is(vm, idx); }

    static void Push(StateRef vm, const Type& obj)
    {
        auto sptr = obj.lock();
        if (sptr) Detail::SmartPush<T>::Push(vm, *sptr.GetCtrl(), *sptr);
        else lua_pushnil(vm);
    }

    static void PrintName(std::ostream& os) { os << TYPE_NAME<T>; }
};

template <typename Class, typename T, T Class::* Member>
BOOST_FORCEINLINE NotNull<SharedPtr<T>> GetRefCountedOwnedMember(Class& cls)
{ return NotNull<SharedPtr<T>>{&cls, &(cls.*Member), true}; }

template <typename Class, typename T, T Class::* Member>
BOOST_FORCEINLINE NotNull<SharedPtr<T>>
GetSmartOwnedMember(const NotNull<SharedPtr<Class>>& cls)
{ return NotNull<SharedPtr<T>>{cls.Get(), &(cls.get()->*Member)}; }

}

#endif
#endif
