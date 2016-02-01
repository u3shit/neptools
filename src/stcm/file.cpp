#include "file.hpp"
#include "header.hpp"
#include "../item.hpp"
#include <boost/assert.hpp>

namespace Stcm
{

File::File(std::shared_ptr<Buffer> buf, size_t offset, size_t len)
{
    auto root = Create<RawItem>(std::move(buf), offset, len);
    auto root_sav = root.get();
    SetRoot(std::move(root));
    HeaderItem::CreateAndInsert(root_sav);
}

}
