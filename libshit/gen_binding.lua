local fname = arg[1]
local fname_out = arg[2]

if not fname or not fname_out then
  io.stderr:write("Usage: gen_binding.lua input_name output_name clang_options...\n")
  os.exit(1)
end

local utils = require("gen_binding.utils")
local parser = require("gen_binding.parser")
local generator = require("gen_binding.generator")

local cl = require("ljclang")
local vr = cl.ChildVisitResult

-- add general options
local args = {
  "-DLIBSHIT_BINDING_GENERATOR",
  "-Isrc",
  "-Ilibshit/src",
  "-Ilibshit/ext/boost",
  "-Ilibshit/ext/ljx/src",
  "-Ilibshit/ext/catch/include",
  "-Ilibshit/ext/brigand/include",
  "-std=c++1z",
  "-stdlib=libc++", -- probably better c++1* support than random gcc headers
  "-Wno-undefined-inline", "-Wno-undefined-internal", -- skip function bodies
  "-Wno-assume"
}

-- hack: -isystem /usr/lib/clang/<version>/include automatically
local clang_ver = cl.clangVersion():match("%d+%.%d+%.%d+")
if clang_ver then
  local path = (os.getenv("PREFIX") or "/usr").."/lib/clang/"..clang_ver
  local f = io.open(path.."/include/stddef.h")
  if f then
    f:close()
    args[#args+1] = "-resource-dir"
    args[#args+1] = path
  end
end

for i=3,#arg do args[#args+1] = arg[i] end

-- let's go
local idx = cl.createIndex()
local tu = idx:parse(fname, args, {"KeepGoing", "SkipFunctionBodies"})
if tu == nil then
  io.stderr:write("\27[31mParse failed\27[0m\n")
  os.exit(-1)
end

for _,d in ipairs(tu:diagnostics()) do
  if d.severity == "CXDiagnostic_Warning" then
    utils.print_warning(d.text)
  elseif d.severity >= "CXDiagnostic_Error" then
    utils.print_error(d.text)
  else
    io.stderr:write(d.text, "\n")
  end
end

-- main enumeration
local function remove_ext(name)
  return name and name:gsub("%.[^.]*$", "")
end

local match_fname = remove_ext(fname)
local fname_map = {}

local check_alias_v = cl.regCursorVisitor(function(c)
  if c:kind() == "AnnotateAttr" and c:name():sub(1, 11) == "alias_file " then
    fname_map[c:location()] = c:name():sub(12)
  end
  return vr.Break
end)

local function parse_filter(c, par)
  c:children(check_alias_v)
  local loc = c:location()
  if fname_map[loc] then loc = fname_map[loc] end
  return remove_ext(loc) == match_fname
end

local classes = parser.parse(tu:cursor(), parse_filter)

local gen = generator.generate(classes)
if fname_out == "-" then
  io.stdout:write(gen)
else
  local f = io.open(fname_out, "wb")
  f:write(gen)
  f:close()
end
os.exit(utils.fail and 1 or 0)
