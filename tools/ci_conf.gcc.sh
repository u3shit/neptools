source tools/ci_conf_linuxcommon.sh

run export CC="$(cd /usr/bin; ls gcc-*.*.* | tail -n1)"
run export CXX="$(cd /usr/bin; ls g++-*.*.* | tail -n1)"
run export AR=gcc-ar
run export LINKFLAGS=-static-libstdc++
