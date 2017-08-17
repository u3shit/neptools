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
  tbl.entries = {}
  tbl.entries_ord = {}
  return tbl
end

local function class_migrate_inheritable(mod, ref)
  if not mod then return end

  for _,v in ipairs{"smart_object", "const"} do
    if ref[v] ~= nil and mod[v] == nil then mod[v] = ref[v] end
  end
end

local function is_lua_class(type)
  if not type then return nil end
  type = type:canonical()
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
  class_migrate_inheritable(tbl.ret, tbl)
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
      class_migrate_inheritable(tbl, lc)
    end
  elseif kind == "AnnotateAttr" then
    local at = utils.is_lua_annotation(c, {c=c})
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
local function default_hidden(c, tbl, parent)
  if not tbl.implicit then return false end
  -- special cases
  local kind = c:kind()
  if kind == "FunctionDecl" then return false end
  if kind == "Constructor" and parent:type():isAbstract() then return true end

  return c:access() ~= "public" or c:isDeleted() or c:isOverride() or
    c:name():sub(1,8) == "operator"
end

-- function handlers begin
local function func_common(c, info, tbl)
  utils.default_arg(tbl, "name", utils.default_name_fun, c)
  tbl.type = "function"

  info.args = tbl.args or collect_args(c)
  info.argsf = function(wrap, pre)
    return utils.type_list(info.args, inst.aliases, wrap, pre)
  end
  info.result_type = tbl.result_type or utils.type_name(c:resultType(), inst.aliases, c)

  if not tbl.order then tbl.order = 0 end
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
    while info.args[i]:type():canonical():name() == "Libshit::Lua::StateRef" do
      i = i+1
    end
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
  if c:type():isNoexcept() then info.ptr_type = info.ptr_type .. " noexcept" end

  if not tbl.value_tmpl then
    tbl.value_tmpl = "static_cast</*$= ptr_type */>(&/*$= name */)"
  end
  return true
end

local function ctor(c, info, tbl)
  if not tbl.value_tmpl then
    tbl.value_tmpl = "&::Libshit::Lua::TypeTraits</*$= class */>::\z
      Make</*$= argsf('LuaGetRef') */>"
  end
  return true
end

local function general_method(c, info, tbl)
  info.ptr_prefix = c:isStatic() and "" or info.class.."::"
  -- retarded compiler error with gcc in some cases if a non overloaded
  -- static member function is casted, if we manually take the address.
  -- not writing & for non static member functions is a compile error...
  info.address = c:isStatic() and "" or "&"
  info.ptr_suffix = c:isConst() and " const" or ""
  if c:type():isNoexcept() then info.ptr_suffix = info.ptr_suffix .. " noexcept" end

  info.template_suffix = tbl.template_params and "<"..tbl.template_params..">" or ""

  -- fake return type with decltype if it's auto, because you can't cast to
  -- function pointer returning auto...
  if info.result_type == "auto" then
    info.decltype_args = info.argsf(function(x) return "std::declval<"..x..">()" end)
    info.result_type = "decltype(std::declval</*$= class */>()./*$= name */\z
      /*$= template_suffix */(/*$= decltype_args */))"
  end
  info.ptr_type = "/*$= result_type*/ (/*$= ptr_prefix */*)(/*$= argsf() */)/*$= ptr_suffix */"
  info.ptr_value = "/*$= address *//*$= use_class */::/*$= name *//*$= template_suffix */"

  if not tbl.value_tmpl then
    tbl.value_tmpl = "static_cast</*$= ptr_type */>(/*$= ptr_value */)"
    if tbl.wrap then
      tbl.value_tmpl = "/*$= tbl.wrap */<" .. tbl.value_tmpl .. ">::Wrap"
    end
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
  tbl.type = "function"

  info.type = utils.type_name(c, inst.aliases)
  info.ptr_type = info.type.." "..info.class.."::*"
  info.value = "&"..info.class.."::"..info.name

  if not tbl.value_tmpl then
    if tbl.get then
      info.key = tbl.get == true and "::Libshit::Lua::GetMember" or tbl.get
    else
      info.key = tbl.set == true and "::Libshit::Lua::SetMember" or tbl.set
    end
    tbl.value_tmpl = "&/*$= key */</*$= class */, /*$= type */, /*$= value */>"
  end
  if not tbl.order then tbl.order = 0 end
  return true
end

local function constant(c, info, tbl)
  if tbl.name == nil then tbl.name = info.name end
  tbl.type = "constant"
  tbl.value_tmpl = "/*$= class */::/*$= name */"
  return true
end

local func_type_handlers = {
  FunctionDecl = { func_common, freestanding_func, class_info },
  Constructor = { func_common, class_info, ctor },
  CXXMethod = { func_common, class_info, general_method },
  FunctionTemplate = { method_tmpl, func_common, class_info, general_method },
  FieldDecl = { class_info, field },
  EnumConstantDecl = { class_info, constant },
}

local function lua_function(c, tbl, parent)
  utils.default_arg(tbl, "hidden", default_hidden, c, tbl, parent)
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
  assert(tbl.type)
  tbl.value_str = template(tbl.value_tmpl, info, c)

  -- store entry
  local ents = tbl.class.entries
  local name = tbl.name

  if tbl.type == "constant" then
    assert(ents[name] == nil, "duplicate constant")

    ents[name] = tbl
    tbl.class.entries_ord[#tbl.class.entries_ord+1] = name
  elseif tbl.type == "function" then
    if not ents[name] then
      ents[name] = { type="function" }
      tbl.class.entries_ord[#tbl.class.entries_ord+1] = name
    end
    assert(ents[name].type == "function", "function&constant with same name?")

    if tbl.index then
      table.insert(ents[name], tbl.index, tbl)
    else
      ents[name][#ents[name]+1] = tbl
    end
  else
    error("unknown tbl.type "..tbl.type)
  end
end

local function parse_method(c, kind, parent)
  local ann = utils.get_annotations(c, {cls=inst.parse_class_class})
  if #ann == 0 then
    if kind == "FieldDecl" then
      ann[1] = { get = true, implicit = true }
      -- no setter if class or field const
      if not inst.parse_class_class.const and not c:type():isConst() then
        ann[2] = { set = true, implicit = true }
      end
    -- only ctors/const methods in const classes. if class not const->everything
    elseif kind == "Constructor" or not inst.parse_class_class.const or
           c:isConst() or c:isStatic() then
      ann[1] = { implicit = true }
    end
  end
  for _,a in ipairs(ann) do
    a.class = inst.parse_class_class
    lua_function(c, a, parent)
  end
end

-- inside class
local parse_class_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "CXXBaseSpecifier" and c:access() == "public" then
    local lc = is_lua_class(c:type())
    if lc then
      local parents = inst.parse_class_class.parents
      parents[#parents+1] = lc
    end
  end

  if kind == "UsingDeclaration" then
    for k,v in pairs(c:referenced():overloadedDecls()) do
      local kind = v:kind()
      if kind ~= "TypeAliasDecl" then
        parse_method(v, kind, c:parent())
      end
    end
  end

  if kind ~= "CXXMethod" and kind ~= "FunctionTemplate" and
     kind ~= "Constructor" and kind ~= "FieldDecl" and
     kind ~= "EnumConstantDecl" then
    return vr.Continue
  end
  if c:name() == "PushLua" then inst.parse_class_class.has_push_lua = true end

  parse_method(c, kind, c:parent())
  return vr.Continue
end)

local function is_enum(type)
  return type:haskind("Enum")
end

local function parse_class(type, class)
  utils.default_arg(class, "is_enum", is_enum, type)
  inst.parse_class_class = class
  class.parents = {}
  type:declaration():children(parse_class_v)
  if class.smart_object and not type:isAbstract() and not class.has_push_lua then
    utils.print_warning("Missing LIBSHIT_DYNAMIC_OBJECT", type:declaration())
  end
  return class
end

-- top level parse
local parse_v = cl.regCursorVisitor(function (c, par)
  if not inst.parse_filter(c, par) then return vr.Continue end
  local kind = c:kind()
  if kind == "Namespace" then return vr.Recurse end

  if (kind == "ClassDecl" or kind == "StructDecl" or kind == "EnumDecl") and
    c:isDefinition() and not c:baseTemplate() then -- ignore fwd decls/templates

    local x = is_lua_class(c:type())
    --print(c:type(), x, x.name)
    if x then
      inst.ret_lua_classes[#inst.ret_lua_classes+1] = parse_class(c:type(), x)
    end
    return vr.Recurse -- support inner classes
  elseif kind == "FunctionDecl" then
    for _,a in ipairs(utils.get_annotations(c, {})) do
      lua_function(c, a)
    end
  elseif kind == "TypeAliasDecl" then
    for _,a in ipairs(utils.get_annotations(c, {})) do
      local t = c:typedefType()
      --print(c:type():declaration():kind())
      utils.add_alias(inst.aliases, t:canonical(), c:name())

      local x = is_lua_class(t)
      if x then
        inst.templates[t] = c:name()
        x.name = a.fullname or x.name.."_"..(a.name or c:name())
        x.template = true
        x.alias = a
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
    local anns = utils.get_annotations(c, {})
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

-- basic bubble sort because lua's table.sort is unstable and I'm too lazy to
-- implement something better. plus we will rarely have more than 5 items...
local function sort(tbl, cmp)
  local n = #tbl
  for i=0,n-1 do
    local change = false
    for j=0,n-2-i do
      if cmp(tbl[j+2], tbl[j+1]) then
        local x = tbl[j+1]
        tbl[j+1] = tbl[j+2]
        tbl[j+2] = x
        change = true
      end
    end
    if not change then return end
  end
end

local function parse(c, filter)
  if not filter then filter = function() return true end end
  inst = { lua_classes = {}, ret_lua_classes = {}, parse_filter = filter,
           is_lua = {}, templates = {}, aliases = {}}

  c:children(parse_v)

  if next(inst.templates) then parse_templates(c) end

  local ret = inst.ret_lua_classes
  for _,c in ipairs(ret) do
    for _,m in pairs(c.entries) do
      sort(m, function(a,b) return a.order < b.order end)
    end
  end

  inst = nil
  return ret
end

return {
  parse = parse
}
