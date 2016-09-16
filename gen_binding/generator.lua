-- -*- poly-lua-c++-lua -*-

local function template_code(str)
  return "')"..str:gsub("\\(.)", "%1"):
    gsub("^=(.*)$", "out:write(tostring(%1))").." out:write('"
end
local function template(inp, out, tbl)
  local ninp = inp:gsub("[\\'\n]", "\\%1"):gsub("//$([^\n]*\\\n)", template_code):
    gsub("/%*$(.-)%*%/", template_code):gsub("out:write%(''%)", "")
  --print("local out = ... out:write('"..ninp.."')")
  local fun = assert(loadstring("local out = ... out:write('"..ninp.."')"))
  setfenv(fun, setmetatable(tbl, {__index=_G}))
  fun(out)
end

local template_str = [=[
// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

//$ for i,cls in ipairs(classes) do
namespace Neptools
{
namespace Lua
{

// class /*$= cls.name */
template<>
void TypeRegister::DoRegister</*$= cls.cpp_name */>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;
    bld
//$   local x = { cls.cpp_name }
//$   for i,v in ipairs(cls.parents) do
//$     if not v.no_inherit then x[#x+1] = v.cpp_name end
//$   end
//$   if x[2] then
        .Inherit</*$= table.concat(x, ", ") */>()
//$   end
//$   if cls.value_object and not cls.smart_object then
        .ValueDtor</*$= cls.cpp_name */>()
//$   end

//$ for _,k in ipairs(cls.methods_ord) do
//$   local v = cls.methods[k]
        .Add<
//$       if v[2] then --overloaded
//$         for i,m in ipairs(v) do
            Overload</*$= m.type_str */, /*$= m.value_str */>/*$= i == #v and '' or ',' */
//$         end
//$       else
            /*$= v[1].type_str */, /*$= v[1].value_str */
//$       end
        >("/*$= k */")
//$ end
        ;
/*$= cls.post_register or "" */
}
static TypeRegister::StateRegister</*$= cls.cpp_name */> reg_/*$= cls.name:gsub("%.", "_") */;

}
}

const char /*$= cls.cpp_name */::TYPE_NAME[] = "/*$= cls.name */";

//$ end
]=]

local function generate(out, classes)
  template(template_str, out, {classes=classes})
end

return { generate = generate }
