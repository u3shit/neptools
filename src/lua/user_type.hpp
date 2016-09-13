#ifndef UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#define UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#pragma once

#include "type_traits.hpp"
#include "function_call.hpp"
#include "../shared_ptr.hpp"
#include <type_traits>

namespace Neptools
{

template <typename T> class NotNull;

namespace Lua
{

template <typename T, typename... Args>
static RetNum ValueObjectCtorWrapper(StateRef vm, Args&&... args)
{
    TypeTraits<T>::Push(vm, std::forward<Args>(args)...);
    return {1};
}

class TypeBuilder
{
public:
    TypeBuilder(StateRef vm, void* type_tag);
    void Done();

    TypeBuilder& Name(const char* name);

    template <typename Deriv, typename... Base>
    TypeBuilder& Inherit()
    {
        InheritHelp<Deriv, Base...>::Do(*this);
        return *this;
    }

    template <typename T>
    TypeBuilder& ValueDtor()
    {
        vm.Push<decltype(&DtorFun<T>), &DtorFun<T>>();
        SetField("__gc");
        return *this;
    }

    template <typename T, T fun>
    TypeBuilder& Add(const char* name)
    {
        vm.Push<T, fun>();
        SetField(name);
        return *this;
    }

    template <typename... Args>
    TypeBuilder& Add(const char* name)
    {
        vm.Push<Args...>();
        SetField(name);
        return *this;
    }

    template <typename T>
    TypeBuilder& Add(const char* name, T&& t)
    {
        vm.Push(std::forward<T>(t));
        SetField(name);
        return *this;
    }

    // low-level, pops value from lua stack
    void SetField(const char* name);

private:
    template <typename Deriv, typename... Base>
    struct InheritHelp;

    template <typename T>
    static void DtorFun(StateRef vm, T& t)
    {
        t.~T();
        lua_pushnil(vm);
        lua_setmetatable(vm, 1);
    }

    void DoInherit(ptrdiff_t offs);

    StateRef vm;
};

class TypeRegister
{
public:
    template <typename Class>
    static void Register(StateRef vm)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);

        void* type_tag = &TYPE_TAG<Class>;
        lua_rawgetp(vm, LUA_REGISTRYINDEX, type_tag);
        if (lua_isnil(vm, -1))
        {
            lua_pop(vm, 1);
            TypeBuilder bld{vm, type_tag};
            bld.Name(TYPE_NAME<Class>);
            DoRegister<Class>(vm, bld);
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
    static void DoRegister(StateRef vm, TypeBuilder& bld);
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
