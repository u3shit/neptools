#ifndef UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#define UUID_365580B3_AF64_4E79_8FC1_35F50DFF840F
#pragma once

#include "base.hpp"
#include <type_traits>

namespace Neptools
{
namespace Lua
{

class TypeBuilder
{
public:
    TypeBuilder(StateRef vm, void* type_tag);
    void Done();

    TypeBuilder& Name(const char* name);

    template <typename... Base>
    TypeBuilder& Inherit()
    {
        InheritHelp<Base...>::Do(*this);
        return *this;
    }

    template <typename Class, typename... Args>
    TypeBuilder& SharedCtor()
    {
        return Add<decltype(&std::make_shared<Class, Args...>),
                   &std::make_shared<Class, Args...>>("new");
    }

    template <typename T, T fun>
    TypeBuilder& Add(const char* name)
    {
        vm.Push<T, fun>();
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
    template <typename... Base>
    struct InheritHelp;

    void DoInherit();

    StateRef vm;
};

class TypeRegister
{
public:
    template <typename Class>
    static void Register(StateRef vm)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);

        void* type_tag = &Class::TYPE_TAG;
        lua_rawgetp(vm, LUA_REGISTRYINDEX, type_tag);
        if (lua_isnil(vm, -1))
        {
            lua_pop(vm, 1);
            TypeBuilder bld{vm, type_tag};
            bld.Name(Class::TYPE_NAME);
            DoRegister<Class>(vm, bld);
            bld.Done();
        }

        NEPTOOLS_LUA_CHECKTOP(vm, top+1);
    }

private:
    template <typename Class>
    static void DoRegister(StateRef vm, TypeBuilder& bld);
};

template<> struct TypeBuilder::InheritHelp<>
{ static void Do(TypeBuilder&) {} };

template <typename Head, typename... Rest>
struct TypeBuilder::InheritHelp<Head, Rest...>
{
    static void Do(TypeBuilder& bld)
    {
        TypeRegister::Register<Head>(bld.vm);
        bld.DoInherit();
        InheritHelp<Rest...>::Do(bld);
    }
};


}
}

#endif
