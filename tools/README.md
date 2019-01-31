The scripts in this folder are used by a jenkins slave to build and test
neptools. To use it you'll need an amd64 sysroot, a qemu image of win 7 (or
later) with ssh, wine, msvc includes+libs, patched clang, gcc, and probably
else. Documetation is mostly non-existing.

Sysroot creation
================

You need docker to run this script, it will place the base sysroot in your
working directory. Readline and its deps (ncurses, tinfo) are only required by
the ljx executble, they're not linked into stcm-editor.

```sh
docker run --rm jimbly/steamrt-amd64-gcc bash -c 'apt-get update >&2 && apt-get -y install libreadline6-dev >&2 && dpkg-query -L libc6 libc6-dev linux-libc-dev libgcc1 gcc-4.6 libreadline6-dev libncurses5-dev libtinfo-dev | grep -E "^(/usr/include/|/usr/lib/|/lib/)" | xargs tar cvh --no-recursion' | tar x
rm usr/lib/x86_64-linux-gnu/lib{readline,ncurses,tinfo}.so
mv usr/lib/x86_64-linux-gnu/lib{*_nonshared,readline,ncurses,tinfo}.a ./
rm usr/lib/x86_64-linux-gnu/*.a
rm -r usr/lib/x86_64-linux-gnu/{gconv,libc}
rm usr/lib/gcc/x86_64-linux-gnu/*/{lto1,lto-wrapper}
mv lib{*_nonshared,readline,ncurses,tinfo}.a usr/lib/x86_64-linux-gnu/
```

To compile libc++, you'll need an [llvm git clone][llvm-git] at 7.0.1 or
manually downloading and unpacking a libc++ and libc++abi tarball to a
directory.

```sh
LLVM="/path/to/llvm/clone"
SYSROOT="/path/to/sysroot"
mkdir tmp
cd tmp
rm *.o
clang++ --sysroot "$SYSROOT" -g -DNDEBUG -D_GNU_SOURCE -D_LIBCPP_BUILDING_LIBRARY -D_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fPIC -fvisibility-inlines-hidden -std=c++11 -ffunction-sections -fdata-sections -O3 -flto=thin -DLIBCXX_BUILDING_LIBCXXABI -nostdinc++ -I "$LLVM"/libcxxabi/include -I "$LLVM"/libcxx/lib -I "$LLVM"/libcxx/include -c "$LLVM"/libcxx/src/*.cpp
clang++ --sysroot "$SYSROOT" -g -D_GNU_SOURCE -D_LIBCPP_DISABLE_EXTERN_TEMPLATE -D_LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS -D_LIBCXXABI_BUILDING_LIBRARY -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fPIC -fvisibility-inlines-hidden -std=c++11 -ffunction-sections -fdata-sections -O3 -flto=thin -nostdinc++ -fstrict-aliasing -funwind-tables -D_DEBUG -I "$LLVM"/libcxxabi/src -I "$LLVM"/libcxxabi/include -I "$LLVM"/libcxx/include -c "$LLVM"/libcxxabi/src/*.cpp
llvm-ar rs libc++.a *.o
mv libc++.a "$SYSROOT"/usr/lib
mkdir -p "$SYSROOT"/usr/include/c++/v1
cp -R "$LLVM"/libcxx{,abi}/include/* "$SYSROOT"/usr/include/c++/v1
rm "$SYSROOT"/usr/include/c++/v1/{CMakeLists.txt,__config_site.in}
```

[llvm-git]: https://github.com/llvm-project/llvm-project-20170507
