local cl = require("ljclang")
local vr = cl.ChildVisitResult
local ffi = require("ffi")

local ret = {}

ret.fail = false

local function print_fancy(color, str, c)
  io.stderr:write(color)
  if c then
    local file, line = c:location()
    io.stderr:write(file, ":", line, ": ", tostring(c), ": ")
  end
  io.stderr:write(str, "\27[0m\n")
end

function ret.print_error(str, c)
  ret.fail = true
  print_fancy("\27[31m", str, c)
end

function ret.print_warning(str, c)
  print_fancy("\27[33m", str, c)
end

function ret.default_name_class(c)
  return c:name():gsub("<.*", ""): -- eat template arguments
    gsub("(%l)(%u)", "%1_%2"):gsub("::", "."):lower()
end

function ret.default_name_fun(c)
  if c:kind() == "Constructor" then return "new" end
  return c:name():gsub("(%l)(%u)", "%1_%2"):lower()
end

function ret.fun_qualified_name(c)
  local tbl = { c:name() }
  local x = c:parent()
  i = 1
  while x and x:kind() == "Namespace" do
    i = i-1
    tbl[i] = x:name()
    x = x:parent()
  end
  return table.concat(tbl, "::", i, 1)
end

function ret.default_arg(tbl, arg, def_fun, ...)
  if tbl[arg] == nil then
    tbl[arg] = def_fun(...)
  end
end

function ret.add_alias(aliases, type, alias)
  aliases[type:name():gsub("[.%[%]*+%-?%%^$]", "%%%0")] = alias:gsub("%%", "%%%%")
end

-- todo: fully qualified name
local function get_type_intname(x)
  if type(x) == "string" then return x
  elseif ffi.istype(cl.Cursor_t, x) then return x:type():name()
  elseif ffi.istype(cl.Type_t, x) then return x:name()
  else error("invalid type parameter") end
end

local function type_name(typ, aliases)
  local n = get_type_intname(typ)
  for k,v in pairs(aliases) do
    n = n:gsub(k, v)
    --print(n, k, v)
  end
  return n:gsub("std::__cxx11::", "std::"):gsub("std::__1::", "std::")
end
ret.type_name = type_name

function ret.type_list(args, aliases, wrap, pre)
  if wrap then
    for i,v in ipairs(args) do
      args[i] = wrap.."<"..type_name(args[i], aliases)..">"
    end
  else
    for i,v in ipairs(args) do
      args[i] = type_name(args[i], aliases)
    end
  end
  local cat = table.concat(args, ", ")
  if pre and cat ~= "" then return ", "..cat end
  return cat
end

function ret.is_lua_annotation(c)
  if c:name():sub(1,4) == "lua{" then
    return assert(loadstring("return "..c:name():sub(4)))()
  end
end

-- get annotation
local get_annot_tbl
local get_annot_v = cl.regCursorVisitor(function (c, par)
  if c:kind() == "AnnotateAttr" then
    local x = ret.is_lua_annotation(c)
    if x then get_annot_tbl[#get_annot_tbl+1] = x end
  end
  return vr.Continue
end)

function ret.get_annotations(c)
  get_annot_tbl = {}
  c:children(get_annot_v)
  return get_annot_tbl
end

ffi.cdef("char* getcwd(char* buf, size_t size)")
function ret.getcwd()
  local siz = 64
  while true do
    local dat = ffi.new("char[?]", siz)
    local ok = ffi.C.getcwd(dat, siz)
    if ok == dat then return ffi.string(dat) end
    if ffi.errno() ~= 34 then error("getcwd failed") end
    siz = siz*2
  end
end

function ret.parse_path(base, path)
  if not path then return {} end
  if path:sub(1,1) ~= '/' then path = base..'/'..path end
  local ret = {}
  for c in path:gmatch("[^/]+") do
    if c == ".." then
      assert(ret[#ret])
      ret[#ret] = nil
    elseif c ~= "." then
      ret[#ret+1] = c
    end
  end
  return ret
end

return ret
