#ifndef UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#define UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#pragma once

#include "../context.hpp"
#include "../../source.hpp"
#include "../../txt_serializable.hpp"

namespace Neptools
{
namespace Stsc
{

class File : public Context, public TxtSerializable
{
public:
    File(Source src);

private:
    void Parse_(Source& src);

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
};

}
}

#endif
