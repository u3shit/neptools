#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "gbnl.hpp"
#include "../../source.hpp"
#include "../../txt_serializable.hpp"
#include "../context.hpp"

namespace Neptools::Stcm
{

  class File final : public Context, public TxtSerializable
  {
    LIBSHIT_DYNAMIC_OBJECT;

    template <typename T>
    using GbnlVectG = std::vector<Libshit::NotNull<Libshit::SmartPtr<T>>>;
  public:
    using GbnlVect = GbnlVectG<GbnlItem>;
    using ConstGbnlVect = GbnlVectG<const GbnlItem>;

    File() = default;
    File(Source src);

    LIBSHIT_NOLUA ConstGbnlVect FindGbnl() const;

    LIBSHIT_LUAGEN(wrap="TableRetWrap")
    GbnlVect FindGbnl();

  protected:
    void Inspect_(std::ostream& os, unsigned indent) const override;

  private:
    void Parse_(Source& src);

    template <typename ItemT, typename GbnlT>
    void FindGbnl_(ItemT& root, GbnlVectG<GbnlT>& vect) const;

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
  };

}
#endif
