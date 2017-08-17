#! /bin/bash

# Run mingw winres as an rc replacement
# The 64-bit version can create 32-bit and 64-bit resource files too

args=()

arch="x86-64"
while (( "$#" )); do
    case "$1" in
        -m32)    arch="i386"       ;;
        -m64)    arch="x86-64"     ;;
        /I*|/D*) args+=("-${1#/}") ;;
        /fo)     args+=("-o")      ;;
        *)       args+=("$1")      ;;
    esac
    shift
done

#set -x
exec x86_64-w64-mingw32-windres -O coff -F pe-$arch "${args[@]}"
