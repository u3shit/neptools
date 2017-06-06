#ifndef UUID_2BA17F45_80C0_4F6E_A84A_1BEAC95A48DC
#define UUID_2BA17F45_80C0_4F6E_A84A_1BEAC95A48DC
#pragma once

#include "base.hpp"
#include "function_call_types.hpp"
#include "../meta_utils.hpp"

namespace Neptools::Lua
{

template <auto Fun, typename Args = FunctionArguments<decltype(Fun)>>
struct TableRetWrap;

template <auto Fun, typename... Args>
struct TableRetWrap<Fun, brigand::list<Args...>>
{
    static RetNum Wrap(StateRef vm, Args&&... args)
    {
        NEPTOOLS_LUA_GETTOP(vm, top);
        const auto& ret = Invoke(Fun, std::forward<Args>(args)...);
        auto size = ret.size();

        lua_createtable(vm, size ? size-1 : 0, 0); //+1
        size_t i = 0;
        for (const auto& it : ret)
        {
            vm.Push(it); //+2
            lua_rawseti(vm, -2, i++); // +1
        }

        NEPTOOLS_LUA_CHECKTOP(vm, top+1);
        return 1;
    }
};

}

#endif
