local reg = debug.getregistry()
local type, rawget, error, format = type, rawget, error, string.format

reg.neptools_new_mt = { __call = function(t, ...) return t.new(...) end }

function reg.neptools_mt_index(mt)
  return function(self, key)
    local v = rawget(mt, key)
    if v then return v end

    if type(key) == "string" then
      v = rawget(mt, format("get_%s", key)) -- apparently format inlines better
      if v then return v(self) end
    end

    v = rawget(mt, "get")
    if v then return v(self, key) end
  end
end

function reg.neptools_mt_newindex(mt)
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
