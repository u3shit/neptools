function do_strip()
{
    local f="$1"
    run objcopy --only-keep-debug "$f" "$f.debug"
    run chmod -x "$f.debug"
    run strip --strip-unneeded -R .comment -R .GCC.command.line -R .gnu_debuglink "$f"
    run objcopy --add-gnu-debuglink="$f.debug" "$f"
}

function after_build()
{
    for f in stcm-editor libshit/ext/luajit-ljx; do
        [[ -f "build/$f" ]] && do_strip "build/$f"
    done
}

function join()
{
    local IFS=$1
    shift
    echo "$*"
}
