#! /bin/bash

cd "$(dirname ${BASH_SOURCE[0]})"

[[ -z $PREFIX ]] || LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"

export LUA_PATH=";;ext/ljclang/?.lua"
export LD_LIBRARY_PATH="ext/ljclang:$LD_LIBRARY_PATH"

src=(src/dumpable src/sink src/source src/txt_serializable
     src/format/cl3 src/format/gbnl
     test/container/ordered_map test/container/parent_list
     test/lua/function_ref test/lua/user_type)

req=()
while (( "$#" )); do
    [[ $1 == -- ]] && break
    req+=("$1")
    src=("${req[@]}")
    shift
done
shift

for i in "${src[@]}"; do
    echo "$i.cpp" >&2
    ${LUAJIT:-luajit} gen_binding.lua $i.cpp $i.binding.hpp "$@"
done
