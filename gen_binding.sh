#! /bin/bash

cd "$(dirname ${BASH_SOURCE[0]})"

[[ -z $PREFIX ]] || LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"

export LUA_PATH=";;ext/ljclang/?.lua"
export LD_LIBRARY_PATH="ext/ljclang:$LD_LIBRARY_PATH"

src=(src/dumpable src/sink src/source src/txt_serializable
     src/format/cl3 src/format/context src/format/eof_item src/format/gbnl
     src/format/item src/format/raw_item
     src/format/stcm/collection_link src/format/stcm/data src/format/stcm/file
     src/format/stcm/gbnl src/format/stcm/header
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

ret=0
for i in "${src[@]}"; do
    echo "$i.cpp" >&2
    ${LUAJIT:-luajit} gen_binding.lua $i.cpp $i.binding.hpp "$@"
    tret=$?
    [[ $tret != 0 ]] && ret=$tret
done
exit $ret
