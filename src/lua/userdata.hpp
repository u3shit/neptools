#ifndef UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#define UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#pragma once

#include "type_traits.hpp"
#include "../shared_ptr.hpp"

namespace Neptools
{
namespace Lua
{

class UserdataBase
{
public:
    UserdataBase(void* obj) noexcept : obj{obj} {}
    UserdataBase(const UserdataBase&) = delete;
    void operator=(const UserdataBase&) = delete;
    virtual void Destroy(StateRef) noexcept = 0;

    void* Get() const noexcept { return obj; }

    template <typename T>
    T& Get(size_t offs) const noexcept
    { return *reinterpret_cast<T*>(static_cast<char*>(obj) + offs); }

protected:
    virtual ~UserdataBase() = default;
    void* obj;
};

/* probably makes no sense
template <typename T>
class ValueUserdata final : public UserdataBase
{
public:
    template <typename... Args>
    ValueUserdata(Args&&... args)
        : UserdataBase{&t}, T{std::forward<Args>(args)...} {}
    virtual void Destroy(StateRef) noexcept override { this->~ValueUserdata(); }

private:
    ~ValueUserdata() = default;
    T t;
};
*/

struct RefCountedUserdataBase : UserdataBase
{
    using UserdataBase::UserdataBase;
    virtual RefCounted* GetCtrl() const noexcept = 0;

    template <typename T>
    NotNull<SharedPtr<T>> GetShared(size_t offs) const noexcept
    { return NotNull<SharedPtr<T>>{GetCtrl(), &Get<T>(offs), true}; }

    void Destroy(StateRef vm) noexcept override;

protected:
    ~RefCountedUserdataBase() = default;
};

class RefCountedUserdata final : public RefCountedUserdataBase
{
public:
    template <typename T>
    RefCountedUserdata(RefCounted* ctrl, T* ptr) noexcept
        : RefCountedUserdataBase{ptr}
    {
        NEPTOOLS_ASSERT(static_cast<void*>(ctrl) == static_cast<void*>(ptr));
        ctrl->AddRef();
    }

    RefCounted* GetCtrl() const noexcept override
    { return static_cast<RefCounted*>(obj); }

private:
    ~RefCountedUserdata() = default;
};

class SharedUserdata final : public RefCountedUserdataBase
{
public:
    SharedUserdata(RefCounted* ctrl, void* ptr) noexcept
        : RefCountedUserdataBase{ptr}, ctrl{ctrl}
    { ctrl->AddRef(); }

    RefCounted* GetCtrl() const noexcept override { return ctrl; }

private:
    ~SharedUserdata() = default;
    RefCounted* ctrl;
};

// specialize
template <typename T, typename Enable = void>
struct IsUserdataObject : std::false_type {};

namespace UserdataDetail
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
struct UserTypeTraits<T, std::enable_if_t<IsUserdataObject<T>::value>>
    : UserdataDetail::TypeTraits {};

namespace UserdataDetail
{

struct UBArgs { UserdataBase* ud; lua_Integer offs; };

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
            static_cast<RefCountedUserdataBase*>(a.ud)->GetCtrl() == a.ud->Get());
        return a.ud->Get<T>(a.offs);
    };
};

template <typename T>
struct TraitsBase<SharedPtr<T>>
{
    using Type = T;
    using Ret = NotNull<SharedPtr<T>>;

    inline static Ret UBGet(UBArgs a)
    {
        return static_cast<RefCountedUserdataBase*>(a.ud)->GetShared<T>(a.offs);
    }
};

UBArgs GetBase(
    StateRef vm, bool arg, int idx, const char* name, void* tag);
UBArgs UnsafeGetBase(StateRef vm, int idx, void* tag);
bool IsBase(StateRef vm, int idx, void* tag);
template <typename Userdata>
void Push(StateRef vm, RefCounted& ctrl, void* ptr, void* tag);

}

template <typename T>
struct UserdataTraits
{
    using Base = UserdataDetail::TraitsBase<T>;
    using BaseType = typename Base::Type;
    using Ret = typename Base::Ret;

    inline static Ret Get(StateRef vm, bool arg, int idx)
    {
        return Base::UBGet(UserdataDetail::GetBase(
            vm, arg, idx, TYPE_NAME<BaseType>, &TYPE_TAG<BaseType>));
    }

    inline static Ret UnsafeGet(StateRef vm, int idx)
    {
        return Base::UBGet(UserdataDetail::UnsafeGetBase(
            vm, idx, &TYPE_TAG<BaseType>));
    }

    inline static bool Is(StateRef vm, int idx)
    { return UserdataDetail::IsBase(vm, idx, &TYPE_TAG<BaseType>); }
};

}
}

#endif
