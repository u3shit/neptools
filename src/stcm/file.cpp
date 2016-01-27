#include "file.hpp"
#include "header.hpp"
#include "../item.hpp"
#include <map>
#include <boost/assert.hpp>

namespace Stcm
{

File::File(std::shared_ptr<Buffer> buf)
{
    SetRoot(Create<RawItem>(std::move(buf)));
    Parse();
}

void File::Parse()
{
    std::map<FilePointer, Item*> pointer_map;
    BOOST_ASSERT(GetRoot()->GetNext() == nullptr &&
                 GetRoot()->GetPosition() == 0 &&
                 dynamic_cast<RawItem*>(GetRoot()));
    pointer_map[0] = GetRoot();

    auto& root = static_cast<RawItem&>(*GetRoot());
    root.Split(0, Create<HeaderItem>(root), pointer_map);
}

}
