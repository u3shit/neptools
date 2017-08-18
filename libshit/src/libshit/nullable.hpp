#ifndef UUID_A4AD7BE1_0609_42C3_ADAB_61F7CED45A4D
#define UUID_A4AD7BE1_0609_42C3_ADAB_61F7CED45A4D
#pragma once

#include <utility>

namespace boost { template <typename T> class intrusive_ptr; }
namespace Libshit
{

  template <typename T> class NotNull;
  template <typename T, template<typename> class Storage>
  class SharedPtrBase;

  // nullable concept
  // value ctor -> puts in null state
  // explicit operator bool() -> is in null state
  // examples: pointer, smart pointers, std::optional

  // make a type nullable
  template <typename T> struct ToNullable;

  template <typename T> struct ToNullable<T&>
  {
    using Type = T*;
    static constexpr T* Conv(T& ref) noexcept { return &ref; }
  };

  template <typename T> struct ToNullable<NotNull<T>>
  {
    using Type = T;
    static T Conv(const NotNull<T>& nn) noexcept { return nn.Get(); }
  };


  // make a type not nullable
  template <typename T> struct ToNotNullable;

  template <typename T> struct ToNotNullable<T*>
  {
    using Type = T&;
    static constexpr T& Conv(T* ptr) noexcept { return *ptr; }
  };

  template <typename T, template<typename> class Storage>
  struct ToNotNullable<SharedPtrBase<T, Storage>>
  {
    using Type = NotNull<SharedPtrBase<T, Storage>>;
    static Type Conv(SharedPtrBase<T, Storage> ptr) noexcept
    { return Type{std::move(ptr)}; }
  };

  template <typename T> struct ToNotNullable<boost::intrusive_ptr<T>>
  {
    using Type = NotNull<boost::intrusive_ptr<T>>;
    static Type Conv(boost::intrusive_ptr<T> ptr) noexcept
    { return Type{std::move(ptr)}; }
  };

}

#endif
