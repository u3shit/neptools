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
    auto root_sav = root.get();
    SetRoot(std::move(root));
    root_sav->Split(root_sav->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({root_sav, 0});
}

GbnlItem& File::FindGbnl()
{
    auto gbnl = FindGbnl_(GetRoot());
    if (!gbnl) NEPTOOLS_THROW(DecodeError{"No GBNL found in STCM"});
    return *const_cast<GbnlItem*>(gbnl);
}

const GbnlItem& File::FindGbnl() const
{
    auto gbnl = FindGbnl_(GetRoot());
    if (!gbnl) NEPTOOLS_THROW(DecodeError{"No GBNL found in STCM"});
    return *gbnl;
}

const GbnlItem* File::FindGbnl_(const Item* root) const
{
    if (!root) return nullptr;

    auto x = dynamic_cast<const Stcm::GbnlItem*>(root);
    if (x) return x;

    x = FindGbnl_(root->GetChildren());
    if (x) return x;

    return FindGbnl_(root->GetNext());
}

void File::WriteTxt_(std::ostream& os) const
{ FindGbnl().WriteTxt(os); }

void File::ReadTxt_(std::istream& is)
{ FindGbnl().ReadTxt(is); }


}
}
