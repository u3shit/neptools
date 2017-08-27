function before()
{
    run ln -nsf /opt/boost libshit/ext/boost
    run rm -f build/stcm-editor build/stcm-editor.exe \
        build/run-tests build/run-tests.exe \
        build/launcher.exe build/lua53.dll build/neptools-server.dll
}
build_opts=(-j2)
tests=(build/run-tests)
