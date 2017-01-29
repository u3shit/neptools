#include "ref_counted_user_data.hpp"

namespace Neptools::Lua
{

void RefCountedUserDataBase::Destroy(StateRef vm) noexcept
{
    ClearCache(vm);
    GetCtrl()->RemoveRef();
    this->~RefCountedUserDataBase();
}

}
