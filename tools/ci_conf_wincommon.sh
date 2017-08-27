run export HOST_CC=gcc
run export HOST_CXX=g++
run export HOST_CFLAGS=
run export HOST_CXXFLAGS=
run export HOST_LINKFLAGS=
config_opts=(--clang-hack --optimize --lua-dll) # pdbs don't work atm

tests=(
    'wine build/run-tests.exe'
    'tools/vmrun.sh /opt/win7-vm/vm.opts build/run-tests.exe build/lua53.dll'
)
