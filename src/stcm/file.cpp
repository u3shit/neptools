#include "file.hpp"
#include "header.hpp"
#include "../item.hpp"
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
    BOOST_ASSERT(GetRoot()->GetNext() == nullptr &&
                 GetRoot()->GetPosition() == 0 &&
                 dynamic_cast<RawItem*>(GetRoot()));

    auto& root = static_cast<RawItem&>(*GetRoot());
    root.Split(0, Create<HeaderItem>(root));
}

}
