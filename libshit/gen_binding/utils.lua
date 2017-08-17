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

function ret.print(str, c)
  print_fancy("", str, c)
end

function ret.default_name_class(c)
  return c:name():gsub("<.*", ""): -- eat template arguments
    gsub("(%u)(%u%l)", "%1_%2"):gsub("(%l)(%u)", "%1_%2"):gsub("::", "."):lower()
end

function ret.default_name_fun(c)
  if c:kind() == "Constructor" then return "new" end
  return c:name():gsub("(%l)(%u)", "%1_%2"):lower()
end

function ret.fun_qualified_name(c)
  local tbl = { c:name() }
  local x = c:parent()
  local i = 1
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

local function escape_gsub_key(str)
  return str:gsub("[.%[%]*+%-?%%^$]", "%%%0")
end

local function escape_gsub_repl(str)
  return str:gsub("%%", "%%%%")
end

local function escape_alias_key(str)
  return "([^a-zA-Z:_])" .. escape_gsub_key(str) .. "([^a-zA-Z0-9_])"
end

local function escape_alias_value(str)
  return "%1" .. escape_gsub_repl(str) .. "%2"
end

function ret.add_alias(aliases, type, alias)
  --print("addalias", type:name(), alias)
  aliases[type:name()] = escape_alias_value(alias)
end

-- extremely hacky way to get fully qualified-ish names
-- totally not optimal, but this is probably the best we can get without
-- hacking even more clang internals or writing a real libtooling tool in C++...

-- collect fully qualified names of the type and all template arguments and all
-- base types
--
-- the latter is required in case of templates:
-- void foo(const typename Bar<T, Asd>::InnerStuff&)
-- we need to turn Bar<...>::InnerStuff into Libshit::Bar...
-- but we only get the InnerStuff type, it's path won't match the template hell
-- calling :canonical() on it will resolve the typedef so it won't work
--
-- tbl: to be filled by this function
--   key=fqn, value=path, a table: {start=integer, [start]="pathitem"...[1]="class name"}
-- t: the type
local collect_ns_cur
local function collect_ns(tbl, t)
  if not t then return end

  local p = t:pointee()
  if p then collect_ns(tbl, p) end

  local cur = t:declaration()
  if cur:kind() == "NoDeclFound" then return end

  for k,v in ipairs(t:templateArguments()) do collect_ns(tbl, v) end
  collect_ns_cur(tbl, cur, 0)
end

collect_ns_cur = function(tbl, cur, prio)
  local path = {cur:name()}
  local repl = {cur:name()}
  local i = 0
  local par = cur:parent()
  while par and par:kind() ~= "TranslationUnit" do
    collect_ns(tbl, par:type())
    path[i] = par:name()
    repl[i] = par:displayName()
    i=i-1
    par = par:parent()
  end

  path.start = i+1
  path.prio = prio
  tbl["::"..table.concat(repl, "::", i+1, 1)] = path
end

-- generate gsub patterns for collected paths
local function gen_gsub(tbl)
  local pats = {}
  local prios = {}
  --print() print("===============================start==========================")

  for k,v in pairs(tbl) do
    local repl = escape_alias_value(k)
    --print(k, table.concat(v, "::", v.start, 1))

    for i=v.start,1 do
      local pat = escape_alias_key(table.concat(v, "::", i, 1))
      if pats[pat] and pats[pat] ~= repl and prios[pat] == v.prio then
        ret.print_error("Ambiguous name?\nPattern: "..pat.."\nOld repl: "..
                        pats[pat].."\nNew repl: "..repl)
      elseif not prios[pat] or prios[pat] < v.prio then
        pats[pat] = repl
        prios[pat] = v.prio
      end
    end
    --print()
  end

  return pats
end

local find_typedefs_tbl
local find_typedefs_v = cl.regCursorVisitor(function (c, par)
  local kind = c:kind()
  if kind == "TypeAliasDecl" then
    collect_ns_cur(find_typedefs_tbl, c, 10)
  --else print("Unhandled", kind, c:name())
  end
  return vr.Continue
end)
local function find_typedefs(tbl, cur)
  if cur == nil then return end
  find_typedefs_tbl = tbl
  cur:children(find_typedefs_v)
  return find_typedefs(tbl, cur:parent())
end

local function apply_rules(str, pats, verbose)
  local ret = "#"..str.."#"
  local chg = true
  while chg do
    chg = false
    for k,v in pairs(pats) do
      if verbose then print(k,"----",v) end
      local n = ret:gsub(k, v)
      if n ~= ret then
        if verbose then print("chg to", n) end
        ret = n
        chg = true
      end
    end
  end
  return ret:sub(2, -2)
end

local function type_name(x, aliases, cur)
  local t
  if type(x) == "string" then return x
  elseif ffi.istype(cl.Cursor_t, x) then
    t = x:type()
    assert(not cur)
    cur = x
  elseif ffi.istype(cl.Type_t, x) then t = x
  else error("invalid type parameter") end

  local tbl = {}
  collect_ns(tbl, t)
  if cur then find_typedefs(tbl, cur, t:name()) end
  local pats = gen_gsub(tbl)

  local tmppats = {}
  for k,v in pairs(aliases) do
    tmppats[escape_alias_key(k)] = v
    tmppats[escape_alias_key(apply_rules(k, pats))] = v
  end
  for k,v in pairs(tmppats) do pats[k] = v end

  --print("\n\nstart", t:name())
  local ret = apply_rules(t:name(), pats, false)

  --print("return", ret)
  return ret:gsub("std::__cxx11::", "std::"):gsub("std::__1::", "std::")
end
ret.type_name = type_name

function ret.type_list(args, aliases, wrap, pre)
  if type(wrap) == "string" then
    local pre = wrap.."<"
    wrap = function(x) return pre..x..">" end
  end
  if not wrap then wrap = function(x) return x end end

  for i,v in ipairs(args) do
    args[i] = wrap(type_name(args[i], aliases))
  end

  local cat = table.concat(args, ", ")
  if pre and cat ~= "" then return ", "..cat end
  return cat
end

local index_G_tbl = {__index=_G}
function ret.is_lua_annotation(c, env)
  if c:name():sub(1,4) == "lua{" then
    local fun = assert(loadstring("return "..c:name():sub(4)))
    setfenv(fun, setmetatable(env, index_G_tbl))
    return fun()
  end
end

-- get annotation
local get_annot_tbl, get_annot_env
local get_annot_v = cl.regCursorVisitor(function (c, par)
  if c:kind() == "AnnotateAttr" then
    local x = ret.is_lua_annotation(c, get_annot_env)
    if x then get_annot_tbl[#get_annot_tbl+1] = x end
  end
  return vr.Continue
end)

function ret.get_annotations(c, env)
  get_annot_tbl = {}
  get_annot_env = env
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

function ret.path_isbase(base, o)
  for i=1,#base do
    if base[i] ~= o[i] then return false end
  end
  return true
end

return ret
