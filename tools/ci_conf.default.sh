function before_build()
{
    run ln -nsf /opt/boost libshit/ext/boost

    run rm -f test*.xml
    # should remove exactly the same items jenkins put into artifacts to prevent
    # stale files ending up in archives
    run rm -f build/stcm-editor build/*.{debug,exe,dll,pdb} \
        build/libshit/ext/luajit-ljx{,.exe,.debug,.pdb} \
        build/libshit/ext/jit/*
}
build_opts=(-j2)

if [[ $mode = rel ]]; then
    mode_arg=(--release)
elif [[ $mode = rel-test ]]; then
    mode_arg=(--release --with-tests)
else
    mode_arg=(--optimize-ext --with-tests)
fi

function build()
{
    [[ $WAF_DISTCLEAN == true ]] && run ./waf distclean
    run ./waf --color=yes configure "${config_opts[@]}" "${mode_arg[@]}" --all-bundle
    run ./waf --color=yes build "${build_opts[@]}"
}

tests=(
    'build/stcm-editor --xml-output=test.xml --test -fc --reporters=libshit-junit,console'
)
