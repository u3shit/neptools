#include "file.hpp"
#include "header.hpp"
#include "../raw_item.hpp"
#include "../eof_item.hpp"

namespace Neptools
{
namespace Stsc
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

}
}
