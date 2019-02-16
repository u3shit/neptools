run clang --version

run export HOST_CC=gcc
run export HOST_CXX=g++
run export HOST_CFLAGS=
run export HOST_CXXFLAGS=
run export HOST_LINKFLAGS=
run export AR=llvm-ar
run export CC=clang
run export CXX=clang++
config_opts=(--lua-dll)

tests=(
    'wine build/stcm-editor.exe --ansi-colors --xml-output=test-wine.xml --test -fc --reporters=junit,console'
    'tools/vmrun.sh /opt/win7-vm/vm.opts test-win7.xml build/stcm-editor.exe build/lua53.dll'
)
