local utils = require("gen_binding.utils")
local template = require("gen_binding.generator").template

local cl = require("ljclang")
local vr = cl.ChildVisitResult

local ffi = require("ffi")

-- find lua exported classes
local inst
local check_lua_class

local function lua_class(c, tbl)
  if tbl.hidden then return false end

  utils.default_arg(tbl, "name", utils.default_name_class, c:type())
  tbl.type = c:type()
  tbl.cpp_name = utils.type_name(c, inst.aliases)
  tbl.methods = {}
  tbl.methods_ord = {}
  return tbl
end

local function is_lua_class(type)
  if not type then return nil end
  local name = type:canonical():name()
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
  elseif kind ~= "ParmDecl" and kind ~= "TypeRef" and kind ~= "AnnotateAttr" then
    utils.print_warning("Unhandled arg type "..c:kind(), c)
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
local function default_hidden(c, tbl)
  if not tbl.implicit then return false end
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

  info.args = tbl.args or collect_args(c)
  info.argsf = function(wrap, pre)
    return utils.type_list(info.args, inst.aliases, wrap, pre)
  end
  info.result_type = tbl.result_type or utils.type_name(c:resultType(), inst.aliases, c)
  return true
end

local function class_info(c, info, tbl)
  info.class = tbl.class.cpp_name
  info.cls = tbl.class
  info.use_class = tbl.use_class or info.class
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
  elseif type(tbl.template_params) == "table" then
    local args = collect_template_args(c)
    for i,s in ipairs(tbl.template_params) do
      utils.add_alias(inst.aliases, args[i], s)
    end
    tbl.template_params = table.concat(tbl.template_params, ", ")
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

  info.ptr_type = info.result_type.." (*)("..info.argsf()..")"

  if not tbl.value_tmpl then
    tbl.type_tmpl = "/*$= ptr_type */"
    tbl.value_tmpl = "&/*$= name */"
  end
  return true
end

local function ctor(c, info, tbl)
  if not tbl.value_tmpl then
    tbl.value_tmpl = [[
&::Neptools::Lua::TypeTraits</*$= class */>::Make</*$= argsf('LuaGetRef') */>]]
    tbl.type_tmpl = nil
  end
  return true
end

local function general_method(c, info, tbl)
  info.ptr_prefix = c:isStatic() and "" or info.class.."::"
  info.ptr_suffix = c:isConst() and " const" or ""

  info.template_suffix = tbl.template_params and "<"..tbl.template_params..">" or ""

  if not tbl.type_tmpl then
    tbl.type_tmpl = "/*$= result_type*/ (/*$= ptr_prefix */*)(/*$= argsf() */)/*$= ptr_suffix */"
  end
  if not tbl.value_tmpl then
    tbl.value_tmpl = "&/*$= use_class */::/*$= name *//*$= template_suffix */"
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

  info.type = utils.type_name(c, inst.aliases)
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
  utils.default_arg(tbl, "hidden", default_hidden, c, tbl)
  if tbl.hidden then return end

  local info = {
    name = utils.fun_qualified_name(c),
    tbl = tbl,

    __index = tbl,
  }

  --print(c:kind(), c:templateKind())
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
  if c:name() == "PushLua" then inst.parse_class_class.has_push_lua = true end

  local ann = utils.get_annotations(c)
  if #ann == 0 then
    if kind == "FieldDecl" then
      ann[1] = { get = true, implicit = true }
      if not c:isConst() then ann[2] = { set = true, implicit = true } end
    else
      ann[1] = { implicit = true }
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
  elseif kind == "TypeAliasDecl" then
    for _,a in ipairs(utils.get_annotations(c)) do
      local t = c:typedefType()
      --print(c:type():declaration():kind())
      utils.add_alias(inst.aliases, t:canonical(), c:name())

      local x = is_lua_class(t)
      if x then
        inst.templates[t] = c:name()
        x.name = x.name.."_"..(a.name or c:name())
        x.template = true
        -- hack!
        x.template_args = t:name():gsub("^[:0-9a-zA-Z ]*<", ""):gsub(">[ *&]*$", "")
        inst.ret_lua_classes[#inst.ret_lua_classes+1] = parse_class(t, x)
      else
        utils.print_error("is not a lua class", c)
      end
    end
  end

  return vr.Continue
end)

local parse_templates2_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "NamespaceRef" or kind == "TypeRef" then return vr.Continue end
  if kind == "TypeAliasDecl" and c:name() == "FakeClass" then
    inst.fake_class = c:typedefType()
    return vr.Continue
  end
  if not inst.fake_class then return vr.Break end

  if kind == "CXXMethod" then
    local anns = utils.get_annotations(c)
    if not anns[1] then anns[1] = { implicit = true } end
    for _,a in ipairs(anns) do
      a.class = is_lua_class(inst.fake_class)
      if a.class then
        a.use_class = utils.type_name(par, inst.aliases)
        lua_function(c, a)
      else
        utils.print_error("FakeClass is not lua class", c)
      end
    end
  end
  return vr.Continue
end)

local parse_templates_v = cl.regCursorVisitor(function (c, par)
  if not inst.parse_filter(c, par) then return vr.Continue end
  local kind = c:kind()
  if kind == "Namespace" then return vr.Recurse end
  if kind == "StructDecl" then
    inst.fake_class = nil
    c:children(parse_templates2_v)
    return vr.Continue
  end

  return vr.Continue
end)

local function parse_templates(c)
  local cwd = utils.getcwd()
  local base = utils.parse_path(cwd, "src")
  local basetest = utils.parse_path(cwd, "test")
  inst.parse_filter = function(c)
    local this = utils.parse_path(cwd, c:location())
    return utils.path_isbase(base, this) or utils.path_isbase(basetest, this)
  end

  c:children(parse_templates_v)
end

local function parse(c, filter)
  if not filter then filter = function() return true end end
  inst = { lua_classes = {}, ret_lua_classes = {}, parse_filter = filter,
           is_lua = {}, templates = {}, aliases = {}}

  c:children(parse_v)

  if next(inst.templates) then parse_templates(c) end

  local ret = inst.ret_lua_classes
  inst = nil
  return ret
end

return {
  parse = parse
}
