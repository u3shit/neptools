#ifndef UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#define UUID_5D65EEDE_B3F7_42A1_8130_9207DE54B2E3
#pragma once

#include "type_traits.hpp"

namespace Neptools::Lua
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
    void ClearCache(StateRef vm) noexcept;
    virtual ~UserDataBase() = default;
    void* obj;
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

    static constexpr bool NEEDS_GC = true;
    static int GcFun(lua_State* vm);
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

template <typename T, typename... Args>
void CreateCachedUserData(StateRef vm, void* ptr, const char* name, Args&&... args)
{
    NEPTOOLS_LUA_GETTOP(vm, top);

    // check cache
    auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, &reftbl); // +1
    NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;
    type = lua_rawgetp(vm, -1, ptr); // +2
    if (type != LUA_TUSERDATA) // no hit
    {
        lua_pop(vm, 1); // +1

        // create object
        auto ud = lua_newuserdata(vm, sizeof(T)); // +1
        auto type = lua_rawgetp(vm, LUA_REGISTRYINDEX, name); // +2
        NEPTOOLS_ASSERT(type == LUA_TTABLE); (void) type;

        new (ud) T{std::forward<Args>(args)...};
        lua_setmetatable(vm, -2); // +1

        // cache it
        lua_pushvalue(vm, -1); // +3
        lua_rawsetp(vm, -3, ptr); // +2
    }
    else
    {
        NEPTOOLS_ASSERT_MSG(
            lua_getmetatable(vm, -1) &&
            lua_rawgetp(vm, -1, name) == LUA_TNUMBER &&
            (lua_pop(vm, 2), true), "Pointer aliasing?");
    }

    lua_remove(vm, -2); // +1 remove reftbl
    NEPTOOLS_LUA_CHECKTOP(vm, top+1);
}

UBArgs GetBase(
    StateRef vm, bool arg, int idx, const char* name);
UBArgs UnsafeGetBase(StateRef vm, int idx, const char* name);
bool IsBase(StateRef vm, int idx, const char* name);

}

template <typename T>
struct UserDataTraits
{
    using Base = UserDataDetail::TraitsBase<T>;
    using BaseType = typename Base::Type;
    using Ret = typename Base::Ret;

    using RawType = BaseType;

    template <bool Unsafe>
    inline static Ret Get(StateRef vm, bool arg, int idx)
    {
        if constexpr (Unsafe && std::is_final_v<T>)
            return Base::UBGet({
                static_cast<UserDataBase*>(lua_touserdata(vm, idx)), 0});
        else if constexpr (Unsafe)
            return Base::UBGet(UserDataDetail::UnsafeGetBase(
                vm, idx, TYPE_NAME<BaseType>));
        else
            return Base::UBGet(UserDataDetail::GetBase(
                vm, arg, idx, TYPE_NAME<BaseType>));
    }

    inline static bool Is(StateRef vm, int idx)
    { return UserDataDetail::IsBase(vm, idx, TYPE_NAME<BaseType>); }

    static constexpr bool INSTANTIABLE = true;
};

}

#endif
