source tools/ci_conf_wincommon.sh

run export CFLAGS='-Xclang -target-cpu -Xclang x86-64 -fms-compatibility-version=18 -imsvc /mnt/msvc/vc12/include -imsvc /mnt/msvc/vc12/win_sdk/include/um -imsvc /mnt/msvc/vc12/win_sdk/include/shared -DCATCH_CONFIG_COLOUR_ANSI'
run export CXXFLAGS="$CFLAGS"
run export LINKFLAGS='/libpath:/mnt/msvc/vc12/lib/amd64 /libpath:/mnt/msvc/vc12/win_sdk/lib/winv6.3/um/x64'
