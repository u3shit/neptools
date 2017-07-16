local setfenv, setmetatable, typename, match, gsub, format, ipairs, error =
  setfenv, setmetatable, typename, string.match, string.gsub, string.format, ipairs, error
assert(setfenv, "no setfenv") -- will fail on plain lua 5.2+

local get_ctx = neptools.item.get_context
local get_children = neptools.item_with_children.get_children

local helpers = {}
-- lazy load table
-- When this script runs, only items and item_with_children is registered.
-- We would need to run this script at the very end of registration phase, but
-- that would fuck up inheritance...
local function gen_helpers(tbl, ns)
  for k,v in pairs(ns) do
    if match(k, "_item$") then
      tbl[gsub(k, "_item$", "")] = v.new
    end
  end
end

local function get_helpers(typename)
  local tbl = helpers[typename]
  if tbl then return tbl end

  tbl = {}
  gen_helpers(tbl, neptools)
  if typename == "neptools.stcm.file" then
    gen_helpers(tbl, neptools.stcm)
    tbl.call = tbl.instruction -- alias
  elseif typename == "neptools.stsc.file" then
    gen_helpers(tbl, neptools.stsc)
  else
    error("unknown type "..typename)
  end

  helpers[typename] = tbl
  return tbl
end

local function build(item, fun)
  local get_label = neptools.context.get_or_create_dummy_label
  local create_label = neptools.context.create_or_set_label

  local ctx = get_ctx(item)
  local children = get_children(item)
  local labels = {}
  local function defer_label(name, offs)
    labels[#labels+1] = { name, offs or 0 }
  end

  local function add(item)
    children:push_back(item)
    for i,v in ipairs(labels) do
      create_label(ctx, v[1], item, v[2])
      labels[i] = nil
    end
    return item
  end

  local tbl = {
    label = defer_label,
    l = function(n) return get_label(ctx, n) end,
    add = add,
    __index = _G,
    __newindex = error,
  }

  -- helpers
  for k,v in pairs(get_helpers(typename(ctx))) do
    tbl[k] = function(...) return add(v(ctx, ...)) end
  end

  setfenv(fun, setmetatable(tbl, tbl))
  fun()
  if labels[1] then
    error(format("label %q after last item", labels[1][1]))
  end
  if ctx == item then ctx:fixup() end -- todo
  return ctx
end

return build
