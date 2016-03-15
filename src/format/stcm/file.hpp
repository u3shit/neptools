#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "../../source.hpp"
#include "../context.hpp"

namespace Neptools
{
namespace Stcm
{

class GbnlItem;
class File : public Context
{
public:
    File(Source src);
    GbnlItem& FindGbnl();

private:
    void Parse_(Source& src);
    GbnlItem* FindGbnl_(Item* root) const;
};

}
}
#endif
