#ifndef UUID_6B456DF2_313B_4FCC_9019_6A38BC7878A3
#define UUID_6B456DF2_313B_4FCC_9019_6A38BC7878A3
#pragma once

#ifdef NEPTOOLS_WITHOUT_LUA

namespace Neptools::Lua
{

struct IntrusiveObject {};

}

#else

#include "../meta.hpp"
#include "user_data.hpp"

#include <boost/intrusive_ptr.hpp>

namespace Neptools::Lua
{

template <typename T>
class IntrusiveUserData final : public UserDataBase
{
public:
    IntrusiveUserData(T* ptr) noexcept
        : UserDataBase{const_cast<std::remove_const_t<T>*>(ptr)}
    { intrusive_ptr_add_ref(ptr); }

    void Destroy(StateRef vm) noexcept
    {
        ClearCache(vm);
        intrusive_ptr_release(static_cast<T*>(obj));
        this->~IntrusiveUserData();
    }
};

class NEPTOOLS_LUAGEN(no_inherit=true) IntrusiveObject {};

namespace UserDataDetail
{
template <typename T>
struct TraitsBase<boost::intrusive_ptr<T>>
{
    using Type = std::remove_const_t<T>;
    using Ret = NotNull<boost::intrusive_ptr<T>>;

    inline static Ret UBGet(UBArgs a) { return Ret{&a.ud->Get<T>(a.offs)}; }
};
}

template <typename T>
constexpr bool IS_INTRUSIVE_OBJECT = std::is_base_of_v<IntrusiveObject, T>;

template <typename T>
struct IsUserDataObject<T, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : std::true_type {};



template <typename T>
struct TypeTraits<T, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : UserDataTraits<T>
{
    static void Push(StateRef vm, T& obj)
    {
        UserDataDetail::CreateCachedUserData<IntrusiveUserData<T>>(
            vm, &obj, &TYPE_TAG<std::remove_const_t<T>>, &obj);
    }
};

template <typename T>
struct TypeTraits<
    NotNull<boost::intrusive_ptr<T>>, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : UserDataTraits<boost::intrusive_ptr<T>>
{
    static void Push(StateRef vm, const NotNull<boost::intrusive_ptr<T>>& obj)
    {
        auto ptr = const_cast<std::remove_const_t<T>*>(obj.get());
        UserDataDetail::CreateCachedUserData<IntrusiveUserData<T>>(
            vm, ptr, &TYPE_TAG<std::remove_const_t<T>>, ptr);
    }
};

template <typename T>
struct TypeTraits<
    boost::intrusive_ptr<T>, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : NullableTypeTraits<boost::intrusive_ptr<T>> {};

}

#endif
#endif
