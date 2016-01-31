#include "file.hpp"
#include "header.hpp"
#include "../item.hpp"
#include <boost/assert.hpp>

namespace Cl3
{

File::File(std::shared_ptr<Buffer> buf)
{
    auto root = Create<RawItem>(std::move(buf));
    auto root_sav = root.get();
    SetRoot(std::move(root));
    HeaderItem::CreateAndInsert(root_sav);
}

}
