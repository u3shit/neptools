function do_strip()
{
    local f="$1"
    run objcopy --only-keep-debug "$f" "$f.debug"
    run chmod -x "$f.debug"
    run strip --strip-unneeded -R .comment -R .GCC.command.line "$f"
    run objcopy --add-gnu-debuglink="$(basename "$f").debug" "$f"
}

function after_build()
{
    [[ $mode = debug ]] || return

    for f in stcm-editor libshit/ext/luajit-ljx; do
        [[ -f "build/$f" ]] && do_strip "build/$f"
    done
}
