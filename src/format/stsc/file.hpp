#ifndef UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#define UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#pragma once

#include "../context.hpp"
#include "../../source.hpp"
#include "../../txt_serializable.hpp"

namespace Neptools::Stsc
{

  enum class LIBSHIT_LUAGEN() Flavor
  {
#define NEPTOOLS_GEN_STSC_FLAVOR(x, ...) \
        x(NOIRE, __VA_ARGS__) \
        x(POTBB, __VA_ARGS__)
#define NEPTOOLS_GEN_ENUM(x,y) x,
        NEPTOOLS_GEN_STSC_FLAVOR(NEPTOOLS_GEN_ENUM,)
#undef NEPTOOLS_GEN_ENUM
  };

  // not constexpr because it only 5 years to implement c++14 in gcc and broken
  // in <gcc-9: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67026
  inline const char* ToString(Flavor f) noexcept
  {
    switch (f)
    {
#define NEPTOOLS_GEN_CASE(x,y) case Flavor::x: return #x;
      NEPTOOLS_GEN_STSC_FLAVOR(NEPTOOLS_GEN_CASE,)
#undef NEPTOOLS_GEN_CASWE
    };
    LIBSHIT_UNREACHABLE("Invalid Flavor value");
  }

  class File final : public Context, public TxtSerializable
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    File(Flavor flavor) : flavor{flavor} {}
    File(Source src, Flavor flavor);

    Flavor flavor;

  protected:
    void Inspect_(std::ostream& os, unsigned indent) const override;

  private:
    void Parse_(Source& src);

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
  };

}

LIBSHIT_ENUM(Neptools::Stsc::Flavor);

#endif
