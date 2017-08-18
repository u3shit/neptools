#ifndef UUID_42B45AF9_5F0C_4F7F_8E80_CD0C4408F8A7
#define UUID_42B45AF9_5F0C_4F7F_8E80_CD0C4408F8A7
#pragma once

#include "assert.hpp"
#include <utility>

namespace Libshit
{

  // construct a null notnull...
  struct EmptyNotNull {};

  template <typename T>
  class NotNull
  {
  public:
    NotNull() = delete;
    NotNull(std::nullptr_t) = delete;
    NotNull(NotNull&) = default;
    NotNull(const NotNull&) = default;
    NotNull(NotNull&&) = default; // NOLINT a moved out NotNull might be null...
    NotNull(EmptyNotNull) noexcept(noexcept(T{})) {}

    template <typename... Args>
    constexpr explicit NotNull(Args&&... args) : t{std::forward<Args>(args)...}
    { LIBSHIT_ASSERT(t); }

    template <typename U>
    NotNull(NotNull<U> o) noexcept(noexcept(T{std::move(o.t)}))
      : t{std::move(o.t)} {}

    NotNull& operator=(const NotNull&) = default;
    NotNull& operator=(NotNull&&) = default; // NOLINT

    // recheck because moving still can break it
    operator T() const { LIBSHIT_ASSERT(t); return t; }
    const T& Get() const noexcept { LIBSHIT_ASSERT(t); return t; }
    decltype(*std::declval<T>()) operator*() const noexcept
    { LIBSHIT_ASSERT(t); return *t; }
    decltype(&*std::declval<T>()) operator->() const noexcept
    { LIBSHIT_ASSERT(t); return &*t; }

    bool operator==(const NotNull& o) const noexcept { return t == o.t; }
    bool operator!=(const NotNull& o) const noexcept { return t != o.t; }

    decltype(auto) get() const { return t.get(); }
    template <typename U,
              typename = std::enable_if_t<std::is_convertible<T, U>::value>>
    operator U() const { LIBSHIT_ASSERT(t); return static_cast<U>(t); }
  private:
    T t;

    template <typename U> friend class NotNull;
  };

  template <typename T>
  NotNull<T> MakeNotNull(T t) noexcept(noexcept(NotNull<T>(std::move(t))))
  { return NotNull<T>(std::move(t)); }
}

#endif
