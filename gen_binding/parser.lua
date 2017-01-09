local utils = require("gen_binding.utils")
local template = require("gen_binding.generator").template

local cl = require("ljclang")
local vr = cl.ChildVisitResult

-- find lua exported classes
local inst
local check_lua_class

local function lua_class(c, tbl)
  if tbl.hidden then return false end

  utils.default_arg(tbl, "name", utils.default_name_class, c:type())
  tbl.type = c:type()
  tbl.cpp_name = utils.type_name(c:type())
  tbl.methods = {}
  tbl.methods_ord = {}
  return tbl
end

local function is_lua_class(type)
  if not type then return nil end
  local name = type:name()
  if inst.lua_classes[name] ~= nil then return inst.lua_classes[name] end

  local tbl = {}
  inst.is_lua[#inst.is_lua+1] = tbl
  type:declaration():children(check_lua_class)
  inst.is_lua[#inst.is_lua] = nil

  if tbl.ret == nil then tbl.ret = false end
  if tbl.ret == true then
    tbl.ret = lua_class(type:declaration(), {})
  end
  if tbl.ret then
    if tbl.value_object then tbl.ret.value_object = true end
    if tbl.smart_object then tbl.ret.smart_object = true end
  end
  inst.lua_classes[name] = tbl.ret

  return tbl.ret
end

check_lua_class = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "CXXBaseSpecifier" then
    local lc = is_lua_class(c:type())
    if lc then
      local tbl = inst.is_lua[#inst.is_lua]
      if tbl.ret == nil then tbl.ret = true end
      if lc.value_object then tbl.value_object = true end
      if lc.smart_object then tbl.smart_object = true end
    end
  elseif kind == "AnnotateAttr" then
    local at = utils.is_lua_annotation(c)
    if at then
      inst.is_lua[#inst.is_lua].ret = lua_class(par, at)
    end
  end
  return vr.Continue
end)

-- get template types
local template_types = {
  TemplateTypeParameter = true,
  NonTypeTemplateParameter = true,
  TemplateTemplateParameter = true,
}
local collect_template_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if template_types[kind] then
    inst.collect_template_tbl[#inst.collect_template_tbl+1] = cl.Cursor(c)
  else
    if kind ~= "ParmDecl" and kind ~= "TypeRef" then
      utils.print_warning("Unhandled arg type "..c:kind(), c)
    end
  end
  return vr.Continue
end)

local function collect_template_args(c)
  inst.collect_template_tbl = {}
  c:children(collect_template_v)
  return inst.collect_template_tbl
end

-- c:arguments() doesn't work on templates...
local collect_args_v = cl.regCursorVisitor(function (c, par)
  if c:kind() == "ParmDecl" then
    inst.collect_args_tbl[#inst.collect_args_tbl+1] = cl.Cursor(c)
  end
  return vr.Continue
end)

local function collect_args(c, app_to)
  inst.collect_args_tbl = app_to or {}
  c:children(collect_args_v)
  return inst.collect_args_tbl
end

-- process lua methods inside classes
local function default_hidden(c)
  -- special cases
  local kind = c:kind()
  if kind == "FunctionDecl" then return false end
  if kind == "Constructor" and c:parent():type():isAbstract() then return true end

  return c:access() ~= "public" or c:isDeleted() or c:isOverride() or
    c:name():sub(1,8) == "operator"
end

-- function handlers begin
local function func_common(c, info, tbl)
  utils.default_arg(tbl, "name", utils.default_name_fun, c)

  info.args = collect_args(c)
  info.argsf = function(wrap, pre) return utils.type_list(info.args, wrap, pre) end
  info.result_type = utils.type_name(c:resultType())
  return true
end

local function class_info(c, info, tbl)
  info.class = tbl.class.cpp_name
  return true
end

local function method_tmpl(c, info, tbl)
  if not tbl.template_params then
    local args = collect_template_args(c)
    if #args ~= 1 or args[1]:name() ~= "Checker" then
      utils.print_error("Must specify template parameters", c)
      return false
    end
    tbl.template_params = "Check::Throw"
  end
  return true
end

local function freestanding_func(c, info, tbl)
  local typ
  if tbl.class then -- user provided
    local x = inst.lua_classes[tbl.class]
    typ = x and x.type
  else
    local i = 1
    while info.args[i]:type():name() == "Lua::StateRef" do i = i+1 end
    typ = info.args[i]:type()
    typ = typ:pointee() or typ -- get rid of &, *
    typ = typ:declaration():type() -- and const, whatever
  end

  tbl.class = is_lua_class(typ)
  if not tbl.class then
    utils.print_error("Invalid base class", c)
    return false
  end

  info.ptr_type = info.result_type.." (*)("..utils.type_list(info.args)..")"

  if not tbl.value_tmpl then
    tbl.type_tmpl = "/*$= ptr_type */"
    tbl.value_tmpl = "&/*$= name */"
  end
  return true
end

local function ctor(c, info, tbl)
  if not tbl.value_tmpl and not tbl.maker then
    if tbl.class.smart_object then
      tbl.maker = "::Neptools::MakeSmart"
    elseif tbl.class.value_object then
      tbl.maker = "::Neptools::Lua::ValueObjectCtorWrapper"
    else
      utils.print_error("Unknown ctor maker", c)
      return false
    end
  end
  if not tbl.value_tmpl then
    tbl.value_tmpl = [[
&/*$= tbl.maker */</*$= class *//*$= argsf('LuaGetRef', true) */>]]
    tbl.type_tmpl = nil
  end
  return true
end

local function general_method(c, info, tbl)
  local ptrpre = c:isStatic() and "" or info.class.."::"
  info.ptr_type = info.result_type.." ("..ptrpre.."*)("..
    utils.type_list(collect_args(c))..")"
  if c:isConst() then info.ptr_type = info.ptr_type.." const" end
  info.value = "&"..info.class.."::"..info.name
  if tbl.template_params then
    info.value = info.value.."<"..tbl.template_params..">"
  end

  if not tbl.value_tmpl then
    tbl.type_tmpl = "/*$= ptr_type */"
    tbl.value_tmpl = "/*$= value */"
  end
  return true
end

local function default_name_field(c, pref)
  return pref..utils.default_name_fun(c)
end

local function field(c, info, tbl)
  if (not tbl.get and not tbl.set) or (tbl.get and tbl.set) then
    utils.print_error("Exactly one of get/set required", c)
    return false
  end

  utils.default_arg(tbl, "name", default_name_field,
                    c, tbl.get and "get_" or "set_")

  info.type = utils.type_name(c:type())
  info.ptr_type = info.type.." "..info.class.."::*"
  info.value = "&"..info.class.."::"..info.name

  if not tbl.value_tmpl then
    if tbl.get then
      info.key = tbl.get == true and "::Neptools::Lua::GetMember" or tbl.get
    else
      info.key = tbl.set == true and "::Neptools::Lua::SetMember" or tbl.set
    end
    tbl.value_tmpl = "&/*$= key */</*$= class */, /*$= type */, /*$= value */>"
  end
  return true
end

local func_type_handlers = {
  FunctionDecl = { func_common, freestanding_func, class_info },
  Constructor = { func_common, class_info, ctor },
  CXXMethod = { func_common, class_info, general_method },
  FunctionTemplate = { func_common, class_info, method_tmpl, general_method },
  FieldDecl = { class_info, field },
}

local function lua_function(c, tbl)
  utils.default_arg(tbl, "hidden", default_hidden, c)
  if tbl.hidden then return end

  local info = {
    name = c:name(),
    tbl = tbl,

    __index = tbl,
  }

  local lst = assert(func_type_handlers[c:kind()])
  for k,v in ipairs(lst) do
    if not v(c, info, tbl) then return end
  end

  assert(tbl.class)
  assert(tbl.value_tmpl)
  tbl.value_str = template(tbl.value_tmpl, info, c)
  tbl.type_str = template(tbl.type_tmpl or "decltype(/*$= tbl.value_str */)", info, c)

  -- store method
  local mets = tbl.class.methods
  local name = tbl.name
  if not mets[name] then
    mets[name] = {}
    tbl.class.methods_ord[#tbl.class.methods_ord+1] = name
  end
  if tbl.index then
    table.insert(mets[name], tbl.index, tbl)
  else
    mets[name][#mets[name]+1] = tbl
  end
end

-- inside class
local parse_class_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "CXXBaseSpecifier" then
    local lc = is_lua_class(c:type())
    if lc then
      local parents = inst.parse_class_class.parents
      parents[#parents+1] = lc
    end
  end

  if kind ~= "CXXMethod" and kind ~= "FunctionTemplate" and
     kind ~= "Constructor" and kind ~= "FieldDecl" then
    return vr.Continue
  end
  if c:name() == "PushLuaObj" then inst.parse_class_class.has_push_lua = true end

  local ann = utils.get_annotations(c)
  if #ann == 0 then
    if kind == "FieldDecl" then
      ann[1] = { get = true }
      if not c:isConst() then ann[2] = { set = true } end
    else
      ann[1] = {}
    end
  end
  for _,a in ipairs(ann) do
    a.class = inst.parse_class_class
    lua_function(c, a)
  end
  return vr.Continue
end)

local function parse_class(type, class)
  inst.parse_class_class = class
  class.parents = {}
  type:declaration():children(parse_class_v)
  if class.smart_object and not type:isAbstract() and not class.has_push_lua then
    utils.print_warning("Missing NEPTOOLS_DYNAMIC_OBJECT", type:declaration())
  end
  return class
end

-- top level parse
local parse_v = cl.regCursorVisitor(function (c, par)
  if not inst.parse_filter(c, par) then return vr.Continue end
  local kind = c:kind()
  if kind == "Namespace" then return vr.Recurse end

  if (kind == "ClassDecl" or kind == "StructDecl") and c:isDefinition() then -- ignore fwd decls
    local x = is_lua_class(c:type())
    --print(c:type(), x, x.name)
    if x then
      inst.ret_lua_classes[#inst.ret_lua_classes+1] = parse_class(c:type(), x)
    end
    return vr.Recurse -- support inner classes
  elseif kind == "FunctionDecl" then
    for _,a in ipairs(utils.get_annotations(c)) do
      lua_function(c, a)
    end
  end

  return vr.Continue
end)

local function parse(c, filter)
  if not filter then filter = function() return true end end
  inst = { lua_classes = {}, ret_lua_classes = {}, parse_filter = filter,
           is_lua = {}}

  c:children(parse_v)
  local ret = inst.ret_lua_classes
  inst = nil
  return ret
end

return {
  parse = parse
}
