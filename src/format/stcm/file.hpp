#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "../../source.hpp"
#include "../../txt_serializable.hpp"
#include "../context.hpp"

namespace Neptools::Stcm
{
  class GbnlItem;

  class File final : public Context, public TxtSerializable
  {
    LIBSHIT_DYNAMIC_OBJECT;

    template <typename T>
    using GbnlVectG = std::vector<Libshit::NotNull<Libshit::SmartPtr<T>>>;

    GbnlItem* first_gbnl = nullptr;

  public:
    File() = default;
    File(Source src);

    LIBSHIT_NOLUA void SetGbnl(GbnlItem& gbnl) noexcept;
    LIBSHIT_NOLUA void UnsetGbnl(GbnlItem& gbnl) noexcept
    { if (first_gbnl == &gbnl) first_gbnl = nullptr; }
    GbnlItem* GetGbnl() const noexcept { return first_gbnl; };

    void Gc() noexcept;

  protected:
    void Inspect_(std::ostream& os, unsigned indent) const override;

  private:
    void Parse_(Source& src);

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
  };

}
#endif
