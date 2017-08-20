#include "eof_item.hpp"
#include "context.hpp"

namespace Neptools
{

  void EofItem::Inspect_(std::ostream& os, unsigned indent) const
  {
    Item::Inspect_(os, indent);
    os << "eof()";
  }

}

#include "eof_item.binding.hpp"
