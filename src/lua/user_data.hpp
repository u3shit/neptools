#ifndef UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#define UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#pragma once

#include "type_traits.hpp"
#include "../shared_ptr.hpp"

namespace Neptools
{
namespace Lua
{

class UserDataBase
{
public:
    UserDataBase(void* obj) noexcept : obj{obj} {}
    UserDataBase(const UserDataBase&) = delete;
    void operator=(const UserDataBase&) = delete;
    virtual void Destroy(StateRef) noexcept = 0;

    void* Get() const noexcept { return obj; }

    template <typename T>
    T& Get(size_t offs) const noexcept
    { return *reinterpret_cast<T*>(static_cast<char*>(obj) + offs); }

protected:
    virtual ~UserDataBase() = default;
    void* obj;
};

/* probably makes no sense
template <typename T>
class ValueUserData final : public UserDataBase
{
public:
    template <typename... Args>
    ValueUserData(Args&&... args)
        : UserDataBase{&t}, T{std::forward<Args>(args)...} {}
    virtual void Destroy(StateRef) noexcept override { this->~ValueUserData(); }

private:
    ~ValueUserData() = default;
    T t;
};
*/

struct RefCountedUserDataBase : UserDataBase
{
    using UserDataBase::UserDataBase;
    virtual RefCounted* GetCtrl() const noexcept = 0;

    template <typename T>
    NotNull<SharedPtr<T>> GetShared(size_t offs) const noexcept
    { return NotNull<SharedPtr<T>>{GetCtrl(), &Get<T>(offs), true}; }

    void Destroy(StateRef vm) noexcept override;

protected:
    ~RefCountedUserDataBase() = default;
};

class RefCountedUserData final : public RefCountedUserDataBase
{
public:
    template <typename T>
    RefCountedUserData(RefCounted* ctrl, T* ptr) noexcept
        : RefCountedUserDataBase{ptr}
    {
        NEPTOOLS_ASSERT(static_cast<void*>(ctrl) == static_cast<void*>(ptr));
        ctrl->AddRef();
    }

    RefCounted* GetCtrl() const noexcept override
    { return static_cast<RefCounted*>(obj); }

private:
    ~RefCountedUserData() = default;
};

class SharedUserData final : public RefCountedUserDataBase
{
public:
    SharedUserData(RefCounted* ctrl, void* ptr) noexcept
        : RefCountedUserDataBase{ptr}, ctrl{ctrl}
    { ctrl->AddRef(); }

    RefCounted* GetCtrl() const noexcept override { return ctrl; }

private:
    ~SharedUserData() = default;
    RefCounted* ctrl;
};

// specialize
template <typename T, typename Enable = void>
struct IsUserDataObject : std::false_type {};

namespace UserDataDetail
{
extern char TAG;

struct TypeTraits
{
    inline static void MetatableCreate(StateRef vm)
    {
        lua_pushboolean(vm, true);
        lua_rawsetp(vm, -2, &TAG);
    }

    static void GcFun(StateRef vm);
};
}

template <typename T>
struct UserTypeTraits<T, std::enable_if_t<IsUserDataObject<T>::value>>
    : UserDataDetail::TypeTraits {};

namespace UserDataDetail
{

struct UBArgs { UserDataBase* ud; lua_Integer offs; };

template <typename T>
struct TraitsBase
{
    using Type = T;
    using Ret = T&;

    inline static Ret UBGet(UBArgs a) { return a.ud->Get<T>(a.offs); }
};

template <typename T>
struct TraitsBase<RefCountedPtr<T>>
{
    using Type = T;
    using Ret = NotNull<RefCountedPtr<T>>;

    inline static Ret UBGet(UBArgs a)
    {
        NEPTOOLS_ASSERT(
            static_cast<RefCountedUserDataBase*>(a.ud)->GetCtrl() == a.ud->Get());
        return Ret{&a.ud->Get<T>(a.offs)};
    };
};

template <typename T>
struct TraitsBase<SharedPtr<T>>
{
    using Type = T;
    using Ret = NotNull<SharedPtr<T>>;

    inline static Ret UBGet(UBArgs a)
    {
        return static_cast<RefCountedUserDataBase*>(a.ud)->GetShared<T>(a.offs);
    }
};

UBArgs GetBase(
    StateRef vm, bool arg, int idx, const char* name, void* tag);
UBArgs UnsafeGetBase(StateRef vm, int idx, void* tag);
bool IsBase(StateRef vm, int idx, void* tag);
template <typename UserData>
void Push(StateRef vm, RefCounted& ctrl, void* ptr, void* tag);

}

template <typename T>
struct UserDataTraits
{
    using Base = UserDataDetail::TraitsBase<T>;
    using BaseType = typename Base::Type;
    using Ret = typename Base::Ret;

    inline static Ret Get(StateRef vm, bool arg, int idx)
    {
        return Base::UBGet(UserDataDetail::GetBase(
            vm, arg, idx, TYPE_NAME<BaseType>, &TYPE_TAG<BaseType>));
    }

    inline static Ret UnsafeGet(StateRef vm, int idx)
    {
        return Base::UBGet(UserDataDetail::UnsafeGetBase(
            vm, idx, &TYPE_TAG<BaseType>));
    }

    inline static bool Is(StateRef vm, int idx)
    { return UserDataDetail::IsBase(vm, idx, &TYPE_TAG<BaseType>); }
};

}
}

#endif
