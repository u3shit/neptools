#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "../../source.hpp"
#include "../../txt_serializable.hpp"
#include "../context.hpp"

namespace Neptools
{
namespace Stcm
{

class GbnlItem;
class File : public Context, public TxtSerializable
{
public:
    File(Source src);
    GbnlItem& FindGbnl();
    const GbnlItem& FindGbnl() const;

private:
    void Parse_(Source& src);
    const GbnlItem* FindGbnl_(const Item* root) const;

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
};

}
}
#endif
