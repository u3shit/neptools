#ifndef UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#define UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#pragma once

#ifndef NEPTOOLS_WITHOUT_LUA

#include "function_call.hpp"
#include "type_traits.hpp"
#include "../shared_ptr.hpp"

#include <type_traits>

namespace Neptools
{

template <typename T> class NotNull;

namespace Lua
{

// When specifying T as template argument for vararg &&... argument,
// std::forward will try to convert it to T&&. If TypeTraits::Get returns a T&,
// it'll fail, even though normally it'd copy it. Thus we need to specify T& in
// this case (or const T&).

namespace Detail
{
template <typename T, typename = void> struct LuaGetRefHlp
{
    using X = decltype(TypeTraits<T>::template Get<false>(
                           std::declval<StateRef>(), false, 0));
    using Type = std::conditional_t<
        !std::is_reference_v<X> || std::is_rvalue_reference_v<X>, T, T&>;
};

template <typename T>
struct LuaGetRefHlp<T, std::enable_if_t<std::is_reference<T>::value>>
{ using Type = T; };

template <typename T>
struct LuaGetRefHlp<T, EnableIfTupleLike<T>> { using Type = T; };
}

template <typename T>
using LuaGetRef = typename Detail::LuaGetRefHlp<T>::Type;

template <typename Class, typename T, T Class::* Member>
BOOST_FORCEINLINE
T& GetMember(Class& cls) { return cls.*Member; }

template <typename Class, typename T, T Class::* member>
BOOST_FORCEINLINE
void SetMember(Class& cls, const T& val) { cls.*member = val; }

class TypeBuilder
{
public:
    TypeBuilder(StateRef vm, void* type_tag, const char* name);
    TypeBuilder(const TypeBuilder&) = delete;
    void operator=(const TypeBuilder&) = delete;

    StateRef GetVm() noexcept { return vm; }
    operator lua_State*() { return static_cast<lua_State*>(vm); }

    template <typename T>
    void Init()
    {
        using UT = UserTypeTraits<T>;
        UT::MetatableCreate(vm);
        if constexpr (UT::NEEDS_GC)
            AddFunction<&UT::GcFun>("__gc");
    }

    void Done();

    template <typename Deriv, typename... Base>
    void Inherit() { InheritHelp<Deriv, Base...>::Do(*this); }

    template <auto... Funs>
    void AddFunction(const char* name)
    {
        vm.PushFunction<Funs...>();
        SetField(name);
    }

    template <typename T>
    void Add(const char* name, T&& t)
    {
        vm.Push(std::forward<T>(t));
        SetField(name);
    }

    // low-level, pops value from lua stack
    // force inlining so the optimizer can optimize out strcmp calls
    BOOST_FORCEINLINE
    void SetField(const char* name)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);

        if (type_tag)
        {
            if (strcmp(name, "get") == 0)      has_get  = true;
            if (strncmp(name, "get_", 4) == 0) has_get_ = true;
            if (strcmp(name, "set") == 0)      has_set  = true;
            if (strncmp(name, "set_", 4) == 0) has_set_ = true;

            lua_pushvalue(vm, -1);
            lua_setfield(vm, -4, name);
            lua_setfield(vm, -2, name);
        }
        else
            lua_setfield(vm, -3, name);

        NEPTOOLS_LUA_CHECKTOP(vm, top-1);
    }


private:
    template <typename Deriv, typename... Base>
    struct InheritHelp;

    static int IsFunc(lua_State* vm) noexcept;

    void DoInherit(ptrdiff_t offs);

    StateRef vm;
    bool has_get_ = false, has_get = false, has_set_ = false, has_set = false;
    void* type_tag;
};

class TypeRegister
{
public:
    template <typename Class>
    static void Register(StateRef vm)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);

        void* type_tag = nullptr;
        bool doit = true;
        if constexpr (TypeTraits<Class>::TYPE_TAGGED)
        {
            type_tag = &TYPE_TAG<Class>;
            lua_rawgetp(vm, LUA_REGISTRYINDEX, type_tag); // +1
            doit = lua_isnil(vm, -1);
            if (doit) lua_pop(vm, 1);
        }
        if (doit)
        {
            TypeBuilder bld{vm, type_tag, TYPE_NAME<Class>};
            bld.Init<Class>();
            DoRegister<Class>(bld);
            bld.Done();
        }

        NEPTOOLS_LUA_CHECKTOP(vm, top+1);
    }

    template <typename... Args>
    static void MultiRegister(StateRef vm)
    {
        using Swallow = int[];
        (void) Swallow{ 0, (Register<Args>(vm), lua_pop(vm, 1), 0)... };
    }

    template <typename... Classes>
    struct StateRegister : State::Register
    {
        StateRegister() : Register{&TypeRegister::MultiRegister<Classes...>} {}
    };

private:
    template <typename Class>
    static void DoRegister(TypeBuilder& bld);
};

template <typename Deriv>
struct TypeBuilder::InheritHelp<Deriv>
{ static void Do(TypeBuilder&) {} };

template <typename Deriv, typename Head, typename... Rest>
struct TypeBuilder::InheritHelp<Deriv, Head, Rest...>
{
    static void Do(TypeBuilder& bld)
    {
        TypeRegister::Register<Head>(bld.vm);
        bld.DoInherit(reinterpret_cast<ptrdiff_t>(static_cast<Head*>(static_cast<Deriv*>(nullptr))));
        InheritHelp<Deriv, Rest...>::Do(bld);
    }
};


}
}

#endif
#endif
