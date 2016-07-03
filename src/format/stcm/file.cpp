#include "file.hpp"
#include "header.hpp"
#include "../item.hpp"
#include "../eof_item.hpp"
#include "gbnl.hpp"

namespace Neptools
{
namespace Stcm
{

File::File(Source src)
{
    AddInfo(&File::Parse_, ADD_SOURCE(src), this, src);
}

void File::Parse_(Source& src)
{
    auto root = Create<RawItem>(src);
    SetupParseFrom(*root);
    root->Split(root->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({root.get(), 0});
}

GbnlItem& File::FindGbnl()
{
    auto gbnl = FindGbnl_(*this);
    if (!gbnl) NEPTOOLS_THROW(DecodeError{"No GBNL found in STCM"});
    return *const_cast<GbnlItem*>(gbnl);
}

const GbnlItem& File::FindGbnl() const
{
    auto gbnl = FindGbnl_(*this);
    if (!gbnl) NEPTOOLS_THROW(DecodeError{"No GBNL found in STCM"});
    return *gbnl;
}

const GbnlItem* File::FindGbnl_(const Item& root) const
{
    auto x = dynamic_cast<const Stcm::GbnlItem*>(&root);
    if (x) return x;

    auto ch = dynamic_cast<const ItemWithChildren*>(&root);
    if (ch)
        for (auto& c : ch->GetChildren())
            if (x = FindGbnl_(c))
                return x;
    return nullptr;
}

void File::WriteTxt_(std::ostream& os) const
{ FindGbnl().WriteTxt(os); }

void File::ReadTxt_(std::istream& is)
{ FindGbnl().ReadTxt(is); }


}
}
