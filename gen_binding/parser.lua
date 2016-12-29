local utils = require("gen_binding.utils")

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

local function lua_method_pre(c, tbl)
  local orig_hidden = tbl.hidden
  utils.default_arg(tbl, "hidden", default_hidden, c)
  if tbl.hidden then return false end

  utils.default_arg(tbl, "name", utils.default_name_fun, c)

  if not tbl.template_params and c:kind() == "FunctionTemplate" then
    local args = collect_template_args(c)
    if #args ~= 1 or args[1]:name() ~= "Checker" then
      if orig_hidden ~= nil then
        utils.print_error("Must specify template parameters", c)
      end
      return false
    end
    tbl.template_params = "Check::Throw"
  end
  return true
end

local function lua_method_post(c, class, tbl)
  tbl.class = class
  if tbl.template_params then
    tbl.value_str = tbl.value_str.."<"..tbl.template_params..">"
  end
  if not tbl.type_str then tbl.type_str = "decltype("..tbl.value_str..")" end

  -- store method
  local mets = class.methods
  local name = tbl.name
  if not mets[name] then
    mets[name] = {}
    class.methods_ord[#class.methods_ord+1] = name
  end
  if tbl.index then
    table.insert(mets[name], tbl.index, tbl)
  else
    mets[name][#mets[name]+1] = tbl
  end
end

local function lua_method(c, class, tbl)
  if not lua_method_pre(c, tbl) then return end

  if c:kind() == "Constructor" then
    if not tbl.maker then
      if class.smart_object then
        tbl.maker = "::Neptools::MakeSmart"
      elseif class.value_object then
        tbl.maker = "::Neptools::Lua::ValueObjectCtorWrapper"
      else
        utils.print_error("Unknown ctor maker", c)
        return nil
      end
    end

    local tl = collect_args(c, {class.cpp_name})
    for i=2,#tl do tl[i] = "LuaGetRef<"..tl[i]:type():name()..">" end
    tbl.value_str = "&"..tbl.maker.."<"..table.concat(tl, ", ")..">"
  else
    local ptrpre = c:isStatic() and "" or class.cpp_name.."::"
    tbl.type_str = utils.type_name(c:resultType()).." ("..ptrpre.."*)("..
      utils.type_list(collect_args(c))..")"
    if c:isConst() then tbl.type_str = tbl.type_str.." const" end
    tbl.value_str = "&"..class.cpp_name.."::"..c:name()
  end

  return lua_method_post(c, class, tbl)
end

local parse_class_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "CXXBaseSpecifier" then
    local lc = is_lua_class(c:type())
    if lc then
      local parents = inst.parse_class_class.parents
      parents[#parents+1] = lc
    end
  end

  if kind ~= "CXXMethod" and kind ~= "FunctionTemplate" and kind ~= "Constructor" then
    return vr.Continue
  end
  if c:name() == "PushLuaObj" then inst.parse_class_class.has_push_lua = true end

  local ann = utils.get_annotations(c)
  if #ann == 0 then ann[1] = {} end
  for _,a in ipairs(ann) do
    lua_method(c, inst.parse_class_class, a)
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

-- parse free-standing functions
local function freestanding_func(c, tbl)
  if not lua_method_pre(c, tbl) then return end

  local typ
  local args = collect_args(c)
  if tbl.class then
    local x = inst.lua_classes[tbl.class]
    typ = x and x.type
  else
    local i = 1
    while args[i]:type():name() == "Lua::StateRef" do i = i+1 end
    typ = args[i]:type()
    typ = typ:pointee() or typ -- get rid of &, *
    typ = typ:declaration():type() -- and const, whatever
  end

  local lc = is_lua_class(typ)
  if not lc then
    utils.print_error("Invalid base class", c)
    return nil
  end

  tbl.type_str = utils.type_name(c:resultType()).." (*)("..
    utils.type_list(args)..")"
  tbl.value_str = "&"..c:name()

  return lua_method_post(c, lc, tbl)
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
  elseif kind == "FunctionDecl" then
    for _,a in ipairs(utils.get_annotations(c)) do
      freestanding_func(c, a)
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
