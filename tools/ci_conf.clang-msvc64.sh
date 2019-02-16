source tools/ci_conf_wincommon.sh

run export CFLAGS="-target x86_64-pc-windows-msvc18 -march=x86-64 \
    -fms-compatibility-version=18 \
    -Xclang -internal-isystem -Xclang /mnt/msvc/vc12/include
    -Xclang -internal-isystem -Xclang /mnt/msvc/vc12/win_sdk/include/um
    -Xclang -internal-isystem -Xclang /mnt/msvc/vc12/win_sdk/include/shared"
run export CXXFLAGS="$CFLAGS -DDOCTEST_CONFIG_COLORS_ANSI"
run export LINKFLAGS="-target x86_64-pc-windows-msvc18 -march=x86-64 \
    -fuse-ld=lld \
    -L/mnt/msvc/vc12/lib/amd64 \
    -L/mnt/msvc/vc12/win_sdk/lib/winv6.3/um/x64"
run export WINRC=x86_64-w64-mingw32-windres
