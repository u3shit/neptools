function before_build()
{
    run ln -nsf /opt/boost libshit/ext/boost
    run rm -f build/stcm-editor build/stcm-editor.exe \
        build/run-tests build/run-tests.exe \
        build/launcher.exe build/lua53.dll build/neptools-server.dll
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
    run ./waf --color=yes configure "${config_opts[@]}" "${mode_arg[@]}" --all-bundle
    run ./waf --color=yes build "${build_opts[@]}"
}

tests=(
    'build/stcm-editor --xml-output=test.xml --test --reporters=junit,console'
)
