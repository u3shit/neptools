#ifndef UUID_C890F042_BE36_4CEF_BB5E_E7F092B4C854
#define UUID_C890F042_BE36_4CEF_BB5E_E7F092B4C854
#pragma once

#include "user_data.hpp"
#include "../shared_ptr.hpp"

namespace Neptools::Lua
{

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

namespace UserDataDetail
{

template <typename T>
struct TraitsBase<RefCountedPtr<T>>
{
    using Type = T;
    using Ret = NotNull<RefCountedPtr<T>>;

    inline static Ret UBGet(UBArgs a)
    {
        NEPTOOLS_ASSERT(
            asserted_cast<RefCountedUserDataBase*>(a.ud)->GetCtrl() == a.ud->Get());
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
        return asserted_cast<RefCountedUserDataBase*>(a.ud)->GetShared<T>(a.offs);
    }
};

}

}

#endif
