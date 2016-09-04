#ifndef UUID_42B45AF9_5F0C_4F7F_8E80_CD0C4408F8A7
#define UUID_42B45AF9_5F0C_4F7F_8E80_CD0C4408F8A7
#pragma once

#include "assert.hpp"
#include <utility>

namespace Neptools
{

template <typename T>
class NotNull
{
public:
    NotNull() = delete;
    NotNull(std::nullptr_t) = delete;
    NotNull(NotNull&) = default;
    NotNull(const NotNull&) = default;
    NotNull(NotNull&&) = default; // a moved out NotNull might be null...

    template <typename... Args>
    constexpr explicit NotNull(Args&&... args) : t{std::forward<Args>(args)...}
    { NEPTOOLS_ASSERT(t); }

    NotNull& operator=(const NotNull&) = default;
    NotNull& operator=(NotNull&&) = default;

    // recheck because moving still can break it
    operator T() const { NEPTOOLS_ASSERT(t); return t; }
    const T& Get() const noexcept { NEPTOOLS_ASSERT(t); return t; }
    auto& operator*() const noexcept { NEPTOOLS_ASSERT(t); return *t; }
    auto* operator->() const noexcept { NEPTOOLS_ASSERT(t); return t.operator->(); }

    bool operator==(const NotNull& o) const noexcept { return t == o.t; }
    bool operator!=(const NotNull& o) const noexcept { return t != o.t; }

private:
    T t;
};

template <typename T>
NotNull<T> MakeNotNull(T t) { return NotNull<T>(std::move(t)); }

}

#endif
