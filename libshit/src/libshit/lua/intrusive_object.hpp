#ifndef UUID_6B456DF2_313B_4FCC_9019_6A38BC7878A3
#define UUID_6B456DF2_313B_4FCC_9019_6A38BC7878A3
#pragma once

#ifdef LIBSHIT_WITHOUT_LUA

namespace Libshit::Lua
{

  struct IntrusiveObject {};

}

#else

#include "type_traits.hpp"
#include "userdata.hpp"
#include "../meta.hpp"

#include <boost/intrusive_ptr.hpp>

namespace Libshit::Lua
{

  class LIBSHIT_LUAGEN(no_inherit=true) IntrusiveObject {};

  template <typename T>
  constexpr bool IS_INTRUSIVE_OBJECT = std::is_base_of_v<IntrusiveObject, T>;



  template <typename T>
  struct TypeTraits<T, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
  {
    static_assert(std::is_final_v<T>);
    using RawType = std::remove_const_t<T>;
    using Ptr = boost::intrusive_ptr<T>;
    static constexpr const char* NAME = TYPE_NAME<RawType>;

    template <bool Unsafe>
    static T& Get(StateRef vm, bool arg, int idx)
    { return *Userdata::GetSimple<Unsafe, Ptr>(vm, arg, idx, NAME); }

    static bool Is(StateRef vm, int idx)
    { return Userdata::IsSimple(vm, idx, NAME); }

    static void Push(StateRef vm, T& obj)
    { Userdata::Cached::Create<Ptr>(vm, &obj, NAME, &obj); }

    static void PrintName(std::ostream& os) { os << TYPE_NAME<T>; }
    static constexpr const char* TAG = TYPE_NAME<T>;
  };

  template <typename T>
  struct TypeTraits<
    NotNull<boost::intrusive_ptr<T>>, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : TypeTraits<T>
  {
    using typename TypeTraits<T>::Ptr;
    using TypeTraits<T>::NAME;

    template <bool Unsafe>
    static NotNull<Ptr> Get(StateRef vm, bool arg, int idx)
    {
      return NotNull<Ptr>{Userdata::GetSimple<Unsafe, Ptr>(
          vm, arg, idx, NAME).get()};
    }

    static void Push(StateRef vm, const NotNull<boost::intrusive_ptr<T>>& obj)
    {
      auto ptr = const_cast<std::remove_const_t<T>*>(obj.get());
      Userdata::Cached::Create<Ptr>(vm, ptr, NAME, ptr);
    }
  };

  template <typename T>
  struct TypeTraits<
    boost::intrusive_ptr<T>, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
    : NullableTypeTraits<boost::intrusive_ptr<T>> {};


  template <typename T>
  struct UserTypeTraits<T, std::enable_if_t<IS_INTRUSIVE_OBJECT<T>>>
  {
    static constexpr bool INSTANTIABLE = true;
    static constexpr bool NEEDS_GC = true;
    static constexpr auto GcFun = Userdata::Cached::GcFun<
      boost::intrusive_ptr<T>, T>;
  };

}

#endif
#endif
