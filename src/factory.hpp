#ifndef UUID_F3B9AB07_80DA_4DCF_AA4F_F6E225EFA110
#define UUID_F3B9AB07_80DA_4DCF_AA4F_F6E225EFA110
#pragma once

#include <vector>

namespace Neptools
{

  template <typename FunT>
  class BaseFactory
  {
  public:
    using Fun = FunT;
    BaseFactory(Fun f) { GetStore().push_back(f); }

  protected:
    using Store = std::vector<Fun>;
    static Store& GetStore()
    {
      static Store store;
      return store;
    }
  };

}
#endif
