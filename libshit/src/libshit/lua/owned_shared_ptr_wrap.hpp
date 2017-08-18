#ifndef UUID_B8B05EAA_92F1_4605_AE60_0067F9F66406
#define UUID_B8B05EAA_92F1_4605_AE60_0067F9F66406
#pragma once

#ifndef LIBSHIT_WITHOUT_LUA

#include "base.hpp"
#include "../meta_utils.hpp"
#include "../shared_ptr.hpp"

namespace Libshit::Lua
{

  template <auto Fun, typename Args = FunctionArguments<decltype(Fun)>>
  struct OwnedSharedPtrWrap;

  template <auto Fun, typename Class, typename... Args>
  struct OwnedSharedPtrWrap<Fun, brigand::list<Class, Args...>>
  {
    using OrigRet = std::remove_reference_t<FunctionReturn<decltype(Fun)>>;
    using NewRet = NotNull<SharedPtr<OrigRet>>;

    static NewRet Wrap(Class&& thiz, Args&&... args)
    {
      return NewRet{
        &thiz,
        &Invoke(Fun, std::forward<Class>(thiz), std::forward<Args>(args)...),
        true};
    }
  };

}

#endif
#endif
