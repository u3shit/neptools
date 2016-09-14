#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "gbnl.hpp"
#include "../../source.hpp"
#include "../../txt_serializable.hpp"
#include "../context.hpp"

namespace Neptools
{
namespace Stcm
{

class File : public Context, public TxtSerializable
{
public:
    File(Source src);
    std::vector<NotNull<SmartPtr<const GbnlItem>>> FindGbnl() const;
    std::vector<NotNull<SmartPtr<GbnlItem>>> FindGbnl();

private:
    void Parse_(Source& src);

    template <typename ItemT, typename GbnlT>
    void FindGbnl_(
        ItemT& root, std::vector<NotNull<SmartPtr<GbnlT>>>& vect) const;

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
};

}
}
#endif
