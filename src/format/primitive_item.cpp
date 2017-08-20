#include "primitive_item.hpp"
#include "context.hpp"

// factory
#include "stcm/data.hpp"

#include "primitive_item.binding.hpp"

namespace Neptools
{
#define NEPTOOLS_GEN(cname, lname, ...) template class __VA_ARGS__
  NEPTOOLS_PRIMITIVE_ITEMS(NEPTOOLS_GEN);
#undef NEPTOOLS_GEN

  template <typename Item, int32_t TypeId>
  static bool CheckFun(Stcm::DataItem& it)
  {
    if (it.type != TypeId || it.offset_unit != 1 || it.field_8 != 1)
      return false;
    auto child = dynamic_cast<RawItem*>(&it.GetChildren().front());
    if (child && child->GetSize() == sizeof(typename Item::Type))
    {
      Item::CreateAndInsert({child, 0});
      return true;
    }
    return false;
  }

  static Stcm::DataFactory reg_int32{CheckFun<Int32Item, 0>};
  static Stcm::DataFactory reg_float{CheckFun<FloatItem, 1>};

}
