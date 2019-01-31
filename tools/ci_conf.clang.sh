source tools/ci_conf_linuxcommon.sh

opt="-march=x86-64 -mtune=generic"
libcxx="-stdlib=libc++"

# sysroot only supported with clang ATM
sysroot=/opt/sysroot_amd64
qsysroot="$(quot "$sysroot")"

run clang --version
run export CC=clang
run export CXX=clang++
run export CFLAGS="--sysroot $qsysroot $opt"
# define: old glibc doesn't know about clang
run export CXXFLAGS="--sysroot $qsysroot $opt $libcxx \
-D__extern_always_inline='extern __always_inline __attribute__((__gnu_inline__))'"
# -lrt only needed with glibc < 2.17
run export LINKFLAGS="--sysroot $qsysroot $libcxx -fuse-ld=lld \
-static-libstdc++ -Wl,--as-needed -lpthread -lrt -Wl,--no-as-needed"

run export PKG_CONFIG_PATH="$(join : "$sysroot"/usr/lib/{,*/}pkgconfig)"
run export PKG_CONFIG_SYSROOT_DIR="$sysroot"


te='build/stcm-editor --xml-output=test'
ts='--test -fc --reporters=junit,console'
ld="LD_LIBRARY_PATH=$(quot "$(join : "$sysroot"/{,usr/}lib/{,*-linux-gnu})") \
$(quot "$sysroot"/lib/*/ld-linux*.so*) $te"
qemu="\$CC -O2 -shared -o tools/qemu_vdso_fix_linux_amd64.so \
tools/qemu_vdso_fix_linux_amd64.c && \
qemu-x86_64 -E LD_PRELOAD=$(quot "$(pwd)/tools/qemu_vdso_fix_linux_amd64.so") \
-E $ld-qemu.xml $ts"

tests=(
    "$te-native.xml $ts"
    "$ld-ld.xml $ts"
    "$qemu"
    "$valgrind"
)
