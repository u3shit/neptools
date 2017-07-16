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
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    File() = default;
    File(Source src);
    NEPTOOLS_NOLUA
    std::vector<NotNull<SmartPtr<const GbnlItem>>> FindGbnl() const;
    NEPTOOLS_LUAGEN(wrap="TableRetWrap")
    std::vector<NotNull<SmartPtr<GbnlItem>>> FindGbnl();

protected:
    void Inspect_(std::ostream& os) const override;

private:
    void Parse_(Source& src);

    template <typename ItemT, typename GbnlT>
    void FindGbnl_(
        ItemT& root, std::vector<NotNull<SmartPtr<GbnlT>>>& vect) const;

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
};

}
#endif
