#ifndef UUID_4B926C10_9735_4A15_BC4B_22DE65201DD1
#define UUID_4B926C10_9735_4A15_BC4B_22DE65201DD1
#pragma once

#include "assert.hpp"
#include "except.hpp"

#include <cstring>
#include <boost/operators.hpp>

namespace Libshit
{

  // like string_view, except it CString=true, it must be zero terminated if
  // !empty() (but can contain embedded zeros)
  template <typename Char, bool CString,
            typename Traits = std::char_traits<Char>>
  class BaseBasicNonowningString
    : boost::totally_ordered<BaseBasicNonowningString<Char, CString, Traits>>,
      boost::totally_ordered<BaseBasicNonowningString<Char, CString, Traits>,
                             const Char*>
  {
  public:
    static constexpr bool ZERO_TERMINATED = CString;
    using traits_type = Traits;
    using value_type = Char;
    using pointer = Char*;
    using const_pointer = const Char*;
    using reference = Char&;
    using const_reference = const Char&;
    using const_iterator = const Char*;
    using iterator = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator = const_reverse_iterator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    using unsigned_value_type = std::make_unsigned_t<Char>;
    using unsigned_pointer = unsigned_value_type*;
    using unsigned_const_pointer = const unsigned_value_type*;
    using unsigned_reference = unsigned_value_type&;
    using unsigned_const_reference = const unsigned_value_type&;

    constexpr BaseBasicNonowningString() noexcept : str{nullptr}, len{0} {}

    BaseBasicNonowningString(const_pointer str) noexcept : str{str}
    { len = str ? Traits::length(str) : 0; }

    BaseBasicNonowningString(unsigned_const_pointer str) noexcept
      : BaseBasicNonowningString{reinterpret_cast<const_pointer>(str)} {}

    constexpr BaseBasicNonowningString(
      const_pointer str, size_type len) noexcept
      : str{str}, len{len}
    {
      LIBSHIT_ASSERT(str != nullptr || len == 0);
      if (CString && str) LIBSHIT_ASSERT(str[len] == '\0');
    }

    constexpr BaseBasicNonowningString(
      unsigned_const_pointer str, size_type len) noexcept
      : BaseBasicNonowningString{reinterpret_cast<const_pointer>(str), len} {}


    template <typename Allocator>
    BaseBasicNonowningString(
      const std::basic_string<Char, Traits, Allocator>& str) noexcept
      : str{str.c_str()}, len{str.size()} {}

    template <size_t N>
    constexpr BaseBasicNonowningString(const Char (&str)[N]) noexcept
      : str{str}, len{N-1} {}

    constexpr const_iterator  begin() const noexcept { return str; }
    constexpr const_iterator cbegin() const noexcept { return str; }
    constexpr const_iterator  end() const noexcept { return str+len; }
    constexpr const_iterator cend() const noexcept { return str+len; }

    constexpr const_reverse_iterator  rbegin() const noexcept { return end(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return end(); }
    constexpr const_reverse_iterator  rend() const noexcept { return begin(); }
    constexpr const_reverse_iterator crend() const noexcept { return begin(); }

    constexpr const_reference operator[](size_type i) const noexcept
    { return str[i]; }
    constexpr const_reference at(size_type i) const
    {
      if (CString ? (i > len) : (i >= len))
        LIBSHIT_THROW(std::out_of_range{"NonowningString"});
      return str[i];
    }

    constexpr unsigned_const_reference uindex(size_type i) const noexcept
    { return reinterpret_cast<unsigned_const_reference>(str[i]); }
    constexpr unsigned_const_reference uat(size_type i) const noexcept
    { return reinterpret_cast<unsigned_const_reference>(at(i)); }

    // undefined if string is empty
    constexpr const_reference front() const noexcept { return str[0]; }
    constexpr const_reference back() const noexcept { return str[len-1]; }
    // can return nullptr or pointer to '\0' in case of empty string
    constexpr const_pointer data() const noexcept { return str; }
    constexpr unsigned_const_pointer udata() const noexcept
    { return reinterpret_cast<unsigned_const_pointer>(str); }

    constexpr size_type size() const noexcept { return len; }
    constexpr size_type length() const noexcept { return len; }
    constexpr size_type max_size() const noexcept
    { return std::numeric_limits<size_type>::max(); }
    constexpr bool empty() const noexcept { return len == 0; }

    constexpr void remove_prefix(size_type n) noexcept { str += n; len -= n; }

    template <typename Allocator = std::allocator<Char>>
    std::basic_string<Char, Traits, Allocator> to_string() const
    { return {str, len}; }

    template <typename Allocator>
    operator std::basic_string<Char, Traits, Allocator>() const
    { return {str, len}; }

    // copy, *find*, substr: too lazy
    template <bool CStringO>
    constexpr int compare(
      BaseBasicNonowningString<Char, CStringO, Traits> o) const noexcept
    {
      auto cmp = Traits::compare(str, o.str, std::min(len, o.len));
      if (cmp == 0) cmp = (len == o.len ? 0 : (len < o.len ? -1 : 1));
      return cmp;
    }
    constexpr int compare(BaseBasicNonowningString o) const noexcept
    { return compare<CString>(o); }
    // too lazy for the other overloads

    template <bool CStringO>
    constexpr bool operator==(
      BaseBasicNonowningString<Char, CStringO, Traits> o) const noexcept
    {
      return len == o.len &&
        (str == o.str || Traits::compare(str, o.str, len) == 0);
    }
    constexpr bool operator==(BaseBasicNonowningString o) const noexcept
    { return operator==<CString>(o); }


    template <bool CStringO>
    constexpr bool operator<(
      BaseBasicNonowningString<Char, CStringO, Traits> o) const noexcept
    { return compare(o) < 0; }
    constexpr bool operator<(BaseBasicNonowningString o) const noexcept
    { return operator< <CString>(o); }

  protected:
    const_pointer str;
    size_type len;
  };

#define X_GEN_OP(op)                                                          \
  template <typename Char, bool CString, typename Traits, typename Allocator> \
  inline bool operator op(                                                    \
    const std::basic_string<Char, Traits, Allocator>& lhs,                    \
    BaseBasicNonowningString<Char, CString, Traits> rhs)                      \
  {                                                                           \
    return BaseBasicNonowningString<Char, CString, Traits>{lhs} op rhs;       \
  }
  X_GEN_OP(==)
  X_GEN_OP(!=)
  X_GEN_OP(<)
  X_GEN_OP(<=)
  X_GEN_OP(>)
  X_GEN_OP(>=)
#undef X_GEN_OP

  template <typename Char, bool CString,
            typename Traits = std::char_traits<Char>>
  class BasicNonowningString;

  template <typename Char, typename Traits>
  class BasicNonowningString<Char, true, Traits> final
    : public BaseBasicNonowningString<Char, true, Traits>
  {
    using Base = BaseBasicNonowningString<Char, true, Traits>;
  public:
    using Base::Base;

    constexpr typename Base::const_pointer c_str() const noexcept
    { return this->data(); }
  };

  template <typename Char, typename Traits>
  class BasicNonowningString<Char, false, Traits> final
    : public BaseBasicNonowningString<Char, false, Traits>
  {
    using Base = BaseBasicNonowningString<Char, false, Traits>;
  public:
    using Base::Base;

    BasicNonowningString(BaseBasicNonowningString<Char, true, Traits> str)
      : Base{str.data(), str.length()} {}

    constexpr void remove_suffix(typename Base::size_type n) noexcept
    { Base::len -= n; }
  };

  using NonowningString = BasicNonowningString<char, true>;
  using NonowningWString = BasicNonowningString<wchar_t, true>;

  using StringView = BasicNonowningString<char, false>;
  using WStringView = BasicNonowningString<wchar_t, false>;

}

#endif
