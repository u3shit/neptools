// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.eof_item
template<>
void TypeRegister::DoRegister<::Neptools::EofItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::EofItem, ::Neptools::Item>();


}
static TypeRegister::StateRegister<::Neptools::EofItem> reg_neptools_eof_item;

}
}


const char ::Neptools::EofItem::TYPE_NAME[] = "neptools.eof_item";

