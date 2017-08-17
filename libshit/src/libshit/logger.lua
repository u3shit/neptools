local log = libshit.log
local type, select, concat, tostring = type, select, table.concat, tostring

local function pack(...)
  return select('#', ...), {...}
end

-- fix l = log("foo") l:info(...) support
local old_check_log = log.check_log
function log.check_log(name, level)
  if type(name) ~= "string" then name = name.name end
  return old_check_log(name, level)
end

local old_raw_log = log.raw_log
function log.raw_log(name, ...)
  if type(name) ~= "string" then name = name.name end
  return old_raw_log(name, ...)
end

-- ... can be a single function or anything else
-- If it's a single function, it's called with zero arguments, and the return
-- values are logged. Otherwise the parameters are converted to strings,
-- concatenated and logged.
function log.log(name, level, ...)
  if type(name) ~= "string" then name = name.name end
  if not old_check_log(name, level) then return end

  local msg = ...
  local n = select('#', ...)
  local tbl
  if n == 1 and type(msg) == 'function' then
    n, tbl = pack(msg())
  else
    tbl = {...}
  end

  for i = 1, n do tbl[i] = tostring(tbl[i]) end
  return old_raw_log(name, level, concat(tbl, '\t')..'\n')
end

-- do not remove returns, tail call optimization removes callframe needed to get
-- proper file and line values in output
function log.err(name, ...) return log.log(name, log.ERROR, ...) end
log.error = log.err
function log.warn(name, ...) return log.log(name, log.WARNING, ...) end
log.warning = log.warn
function log.info(name, ...) return log.log(name, log.INFO, ...) end
function log.debug(name, level, ...)
  assert(0 <= level and level < 5, "invalid debug level")
  return log.log(name, level, ...)
end

log.__index = log
function log.new(name)
  return setmetatable({name=name}, log)
end

setmetatable(log, debug.getregistry().libshit_new_mt)
