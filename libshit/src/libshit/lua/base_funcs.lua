local reg = debug.getregistry()
local type, rawget, error, format, getmetatable =
  type, rawget, error, string.format, debug.getmetatable

-- foo(...) -> foo.new(...)
reg.libshit_new_mt = { __call = function(t, ...) return t.new(...) end }
-- foo("bar", ...) -> foo.new_bar(...)
local function tagged_new(t, name, ...)
  return t[format("new_%s", name)](...)
end
reg.libshit_tagged_new_mt = { __call = tagged_new }

-- __index for classes with "get_*" and maybe "get"
function reg.libshit_mt_index(mt)
  return function(self, key)
    local v = rawget(mt, key)
    if v ~= nil then return v end

    if type(key) == "string" then
      v = rawget(mt, format("get_%s", key)) -- apparently format inlines better
      if v then return v(self) end
    end

    v = rawget(mt, "get")
    if v then return v(self, key) end
  end
end

-- __index for classes with "get", but no "get_*"
function reg.libshit_mt_index_light(mt)
  return function(self, key)
    local v = rawget(mt, key)
    if v ~= nil then return v end

    return rawget(mt, "get")(self, key)
  end
end

-- __newindex for classes with "set_*" and maybe "set"
function reg.libshit_mt_newindex(mt)
  return function(self, key, value)
    if type(key) == "string" then
      local v = rawget(mt, format("set_%s", key))
      if v then return v(self, value) end
    end

    local v = rawget(mt, "set")
    if v then return v(self, key, value) end

    -- bail out
    error(format("attempt to set invalid key %q", key))
  end
end

-- no light: just set __newindex to "set" function

-- our ipairs, counting from 0
local function cb(container, i)
  i = i+1
  local r = container[i]
  if r ~= nil then return i, r end
end

function reg.libshit_ipairs(container)
  return cb, container, -1
end

-- like type() but checks metatable for __name
function typename(t)
  local mt = getmetatable(t)
  if mt then
    local n = mt.__name
    if n then return n end
  end
  return type(t)
end
