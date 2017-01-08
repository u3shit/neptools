local cl = require("ljclang")
local vr = cl.ChildVisitResult

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
  return c:name():gsub("(%l)(%u)", "%1_%2"):gsub("::", "."):lower()
end

function ret.default_name_fun(c)
  if c:kind() == "Constructor" then return "new" end
  return c:name():gsub("(%l)(%u)", "%1_%2"):lower()
end

function ret.default_arg(tbl, arg, def_fun, ...)
  if tbl[arg] == nil then
    tbl[arg] = def_fun(...)
  end
end

-- todo: fully qualified name
function ret.type_name(type)
  return type:name()
end

function ret.type_list(args, wrap, pre)
  if wrap then
    for i,v in ipairs(args) do args[i] = wrap.."<"..args[i]:type():name()..">" end
  else
    for i,v in ipairs(args) do args[i] = args[i]:type():name() end
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

return ret
