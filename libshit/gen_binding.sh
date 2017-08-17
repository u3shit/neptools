#! /bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

[[ -z $PREFIX ]] || LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"

export LUA_PATH="$DIR/?.lua;$DIR/ext/ljclang/?.lua"
export LD_LIBRARY_PATH="$DIR/ext/ljclang:$LD_LIBRARY_PATH"

src+=("$DIR"/test/lua/function_ref "$DIR"/test/lua/user_type)

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
    ${LUAJIT:-luajit} "$DIR/gen_binding.lua" $i.cpp $i.binding.hpp "$@"
    tret=$?
    [[ $tret != 0 ]] && ret=$tret
done
exit $ret
