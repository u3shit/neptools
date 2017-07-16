#ifndef UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#define UUID_CACA9E02_5122_4C09_9463_73AD33BA5802
#pragma once

#include "../context.hpp"
#include "../../source.hpp"
#include "../../txt_serializable.hpp"

namespace Neptools::Stsc
{

class File final : public Context, public TxtSerializable
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    File(Source src);

protected:
    void Inspect_(std::ostream& os) const override;

private:
    void Parse_(Source& src);

    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;
};

}

#endif
