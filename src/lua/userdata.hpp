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

struct RefCountedUserdataBase : UserdataBase
{
    using UserdataBase::UserdataBase;
    virtual RefCounted* GetCtrl() const noexcept = 0;

    template <typename T>
    SharedPtr<T> GetShared(size_t offs) const noexcept
    { return {GetCtrl(), &Get<T>(offs), true}; }

    void Destroy(StateRef vm) noexcept override
    {
        auto ctrl = GetCtrl();
        ClearCache(vm, ctrl);
        ctrl->RemoveRef();
        this->~RefCountedUserdataBase();
    }

protected:
    static void ClearCache(StateRef vm, RefCounted* ctrl)
    {
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
        NEPTOOLS_ASSERT(type == LUA_TTABLE);
        lua_pushnil(vm); // +2
        lua_rawsetp(vm, -2, ctrl); // +1
        lua_pop(vm, 1); // 0
    }

    ~RefCountedUserdataBase() = default;
};

template <typename T>
class RefCountedUserdata final : public RefCountedUserdataBase
{
public:
    RefCountedUserdata(RefCounted* ctrl, T* ptr) noexcept
        : RefCountedUserdataBase{ctrl}
    {
        NEPTOOLS_ASSERT(ctrl == ptr);
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

template <typename T>
struct UserdataTraits
{
    static T& Get(StateRef vm, bool arg, int idx)
    {
        if (!lua_getmetatable(vm, idx)) // +1
            vm.TypeError(arg, TYPE_NAME<T>, idx);
        lua_rawgetp(vm, -1, &TYPE_TAG<T>); // +2

        int isvalid;
        auto offs = lua_tointegerx(vm, -1, &isvalid);
        lua_pop(vm, 2); // 0
        if (!isvalid) vm.TypeError(arg, TYPE_NAME<T>, idx);

        auto ud = static_cast<UserdataBase*>(lua_touserdata(vm, idx));
        NEPTOOLS_ASSERT(ud);

        return ud->Get<T>(offs);
    }

    static T& UnsafeGet(StateRef vm, int idx)
    {
        lua_getmetatable(vm, idx); // +1
        lua_rawgetp(vm, -1, &TYPE_TAG<T>); // +2
        auto offs = lua_tointeger(vm, -1);
        lua_pop(vm, 2); // 0

        auto ud = static_cast<UserdataBase*>(lua_touserdata(vm, idx));
        return ud->Get<T>(offs);
    }

    static bool Is(StateRef vm, int idx)
    {
        if (!lua_getmetatable(vm, idx)) // +1
            return false;
        auto type = lua_rawgetp(vm, -1, &TYPE_TAG<T>); // +2
        lua_pop(vm, 2); // 0
        return type == LUA_TNUMBER;
    }
};

}
}

#endif
