#ifndef UUID_3EC30B5B_ABF6_4EFD_94F4_CC95DF4F9A2D
#define UUID_3EC30B5B_ABF6_4EFD_94F4_CC95DF4F9A2D
#pragma once

#include <cstring>
#include <string>
#include <algorithm>
#include <boost/operators.hpp>

#include "lua/type_traits.hpp"

namespace Libshit
{

  template <size_t N>
  class FixedString :
    boost::totally_ordered<FixedString<N>,
                           boost::totally_ordered<FixedString<N>, const char*>>
  {
  public:
    FixedString() = default;
    explicit FixedString(const char* cstr) { *this = cstr; }
    explicit FixedString(const std::string& sstr) { *this = sstr; }

    FixedString& operator=(const char* cstr)
    { strncpy(str, cstr, N-1); str[N-1] = '\0'; return *this; }

    FixedString& operator=(const std::string& sstr)
    {
      auto copied = std::min(sstr.size(), N-1);
      memcpy(str, sstr.data(), copied);
      memset(str+copied, 0, N-copied);
      return *this;
    }

    bool is_valid() const noexcept
    {
      auto len = strnlen(str, N-1);
      return str[len] == '\0';
    }

    char& operator[](size_t i) noexcept { return str[i]; }
    const char& operator[](size_t i) const noexcept { return str[i]; }

    const char* data() const noexcept { return str; }
    const char* c_str() const noexcept { return str; }

    bool empty() const noexcept { return str[0] == 0; }
    size_t size() const noexcept { return strnlen(str, N); }
    size_t length() const noexcept { return strnlen(str, N); }
    size_t max_size() const noexcept { return N-1; }

    bool operator==(const FixedString& o) const noexcept
    { return strncmp(str, o.str, N) == 0; }
    bool operator<(const FixedString& o) const noexcept
    { return strncmp(str, o.str, N) < 0; }

    bool operator==(const char* o) const noexcept
    { return strcmp(str, o) == 0; }
    bool operator<(const char* o) const noexcept
    { return strcmp(str, o) < 0; }

    template <size_t M>
    bool operator==(const FixedString<M>& o) const noexcept
    { return strcmp(str, o.str) == 0; }
    template <size_t M>
    bool operator<=(const FixedString<M>& o) const noexcept
    { return strcmp(str, o.str) <= 0; }
  private:
    char str[N];
  };

  template <size_t N>
  inline std::ostream& operator<<(std::ostream& os, const FixedString<N>& str)
  { return os << str.data(); }

  // template <size_t N>
  // inline std::istream& operator>>(std::istream& is, FixedString<N>& str)
  // { is.width(N-1); return is >> &str[0]; }

#ifndef LIBSHIT_WITHOUT_LUA
  template <size_t N>
  struct Lua::TypeTraits<FixedString<N>>
  {
    template <bool Unsafe>
    static FixedString<N> Get(StateRef vm, bool arg, int idx)
    {
      size_t len;
      auto str = lua_tolstring(vm, idx, &len);
      if (!Unsafe && BOOST_UNLIKELY(!str))
        vm.TypeError(arg, TYPE_NAME<const char*>, idx);
      if (BOOST_UNLIKELY(len >= N))
      {
        std::stringstream ss;
        ss << "string too long (" << len << " vs " << (N-1) << " max)";
        vm.GetError(arg, idx, ss.str().c_str());
      }

      return FixedString<N>{str};
    }

    static bool Is(StateRef vm, int idx)
    { return lua_type(vm, idx) == LUA_TSTRING; }

    static void Push(StateRef vm, const FixedString<N>& str)
    { lua_pushstring(vm, str.c_str()); }
  };
#endif

}
#endif
