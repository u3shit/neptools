#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "../../source.hpp"
#include "../context.hpp"

namespace Stcm
{

class File : public Context
{
public:
    File(Source src);
};

}
#endif
