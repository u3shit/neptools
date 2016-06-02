#ifndef UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#define UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#pragma once

#include "../context.hpp"
#include "../../source.hpp"

namespace Neptools
{
namespace Stsc
{

class File : public Context
{
public:
    File(Source src);

private:
    void Parse_(Source& src);
};

}
}

#endif
