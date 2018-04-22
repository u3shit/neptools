#ifndef UUID_A2E491BD_BA3E_4C92_B931_72017B89C33C
#define UUID_A2E491BD_BA3E_4C92_B931_72017B89C33C
#pragma once

#include <libshit/assert.hpp>
#include <libshit/meta.hpp>
#include <libshit/lua/type_traits.hpp>

#include <boost/endian/conversion.hpp>

namespace Neptools
{
  enum class LIBSHIT_LUAGEN() Endian
  {
    BIG, LITTLE,
  };

  // not constexpr because gcc is retarded
  inline boost::endian::order ToBoost(Endian e) noexcept
  {
    switch (e)
    {
    case Endian::BIG: return boost::endian::order::big;
    case Endian::LITTLE: return boost::endian::order::little;
    }
    LIBSHIT_UNREACHABLE("Invalid Endian value");
  }

  inline const char* ToString(Endian e) noexcept
  {
    switch (e)
    {
    case Endian::BIG: return "BIG";
    case Endian::LITTLE: return "LITTLE";
    }
    LIBSHIT_UNREACHABLE("Invalid Endian value");
  }

  template <typename T>
  [[nodiscard]] inline T ToNativeCopy(T t, Endian e) noexcept
  {
    return boost::endian::conditional_reverse(
      std::move(t), ToBoost(e), boost::endian::order::native);
  }

  template <typename T>
  [[nodiscard]] inline T FromNativeCopy(T t, Endian e) noexcept
  {
    return boost::endian::conditional_reverse(
      std::move(t), ToBoost(e), boost::endian::order::native);
  }

  template <typename T>
  inline void ToNative(T& t, Endian e) noexcept
  {
    boost::endian::conditional_reverse_inplace(
      t, ToBoost(e), boost::endian::order::native);
  }

  template <typename T>
  inline void FromNative(T& t, Endian e) noexcept
  {
    boost::endian::conditional_reverse_inplace(
      t, ToBoost(e), boost::endian::order::native);
  }

}

LIBSHIT_ENUM(Neptools::Endian);

#endif
