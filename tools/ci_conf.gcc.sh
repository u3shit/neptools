source tools/ci_conf_linuxcommon.sh

opt="-march=x86-64 -mtune=generic"

run export CC="$(cd /usr/bin; ls gcc-*.*.* | tail -n1)"
run export CXX="$(cd /usr/bin; ls g++-*.*.* | tail -n1)"
run export AR=gcc-ar
run export CFLAGS="$opt"
run export CXXFLAGS="$opt"
run export LINKFLAGS="$opt -static-libstdc++"

run $CC --version
