local reg = debug.getregistry()

local function cb(container, i)
  local r = container[i]
  if r then return i, r end
end

function reg.neptools_ipairs(container)
  return cb, container, 0
end
