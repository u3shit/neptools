-- -*- poly-lua-c++-lua -*-

local setfenv, getfenv, setmetatable, concat =
  setfenv, getfenv, setmetatable, table.concat

local utils = require("gen_binding.utils")

local cache = {}
setmetatable(cache, {__mode="v"})
local index_G_tbl = {__index=_G}

local template_tostring
local function template_code(str)
  return "'"..str:gsub("\\(.)", "%1"):
    gsub("^=(.*)$", "__tmpl_out[#__tmpl_out+1]=__tmpl_tos(%1)")..
    " __tmpl_out[#__tmpl_out+1]='"
end
local function template(inp, tbl, c)
  local fun = cache[inp]
  if not fun then
    local ninp = inp:gsub("[\\'\n]", "\\%1"):
      gsub("//$([^\n]*\\\n)", template_code):
      gsub("/%*$(.-)%*%/", template_code)
    ninp = "local __tmpl_tos, __tmpl_concat = ... return function() "..
      "local __tmpl_out = {} __tmpl_out[#__tmpl_out+1]='"..ninp..
      "' return __tmpl_concat(__tmpl_out) end"
    ninp = ninp:gsub("out%[%#out%+1%]%=''", "")
    --print(ninp)
    local err
    fun, err = loadstring(ninp)
    fun = fun and fun(template_tostring, concat)
    if not fun then
      utils.print_error("Invalid template: "..err, c)
      return
    end
    cache[inp] = fun
  end

  setfenv(fun, setmetatable(tbl, index_G_tbl))
  return fun()
end

template_tostring = function(x, env)
  return template(tostring(x), env or getfenv(2))
end

local template_str = [=[
// Auto generated code, do not edit. See gen_binding in project root.
#ifndef LIBSHIT_WITHOUT_LUA
#include <libshit/lua/user_type.hpp>

//$ for i,cls in ipairs(classes) do
//$   if cls.is_enum then
const char ::Libshit::Lua::TypeName</*$= cls.cpp_name */>::TYPE_NAME[] =
  "/*$= cls.name */";
//$   else
/*$ if cls.template then */template <>/*$ end */
const char /*$= cls.cpp_name */::TYPE_NAME[] = "/*$= cls.name */";
//$   end
//$ end

namespace Libshit::Lua
{
//$ for i,cls in ipairs(classes) do

  // class /*$= cls.name */
  template<>
  void TypeRegisterTraits</*$= cls.cpp_name */>::Register(TypeBuilder& bld)
  {
//$   local x = { cls.cpp_name }
//$   for i,v in ipairs(cls.parents) do
//$     if not v.no_inherit then x[#x+1] = v.cpp_name end
//$   end
//$   if x[2] then
    bld.Inherit</*$= table.concat(x, ", ") */>();
//$   end

//$   for _,k in ipairs(cls.entries_ord) do
//$     local v = cls.entries[k]
//$     if v.type == "constant" then
    bld.Add("/*$= k */", /*$= v.value_str */);
//$     else
    bld.AddFunction<
//$       for i,m in ipairs(v) do
      /*$= m.value_str *//*$= i == #v and '' or ',' */
//$       end
    >("/*$= k */");
//$     end
//$   end
/*$= cls.post_register or "", cls */
  }
  static TypeRegister::StateRegister</*$= cls.cpp_name */> reg_/*$= (cls.name:gsub("%.", "_")) */;
//$ end

}
#endif
]=]

local function generate(classes)
  return assert(template(template_str, {classes=classes}), "Generate failed")
end

return { template = template, generate = generate }
