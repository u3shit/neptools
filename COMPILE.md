Note: all of the following assumes you're on Linux. If you're on Windows, have
fun figuring things out...

You'll need python (for the compilation script), a C++14 compiler and boost-1.60
compiled with c++14 support. Most likely you'll have a C++98 ABI version, which
means it'll probably compile but crash randomly when run. Refer to the next
section how to obtain a correct boost version. If you have them, you can just
run:

```
./waf configure
./waf
```

You can specify compile flags on configure:

```
CXX=g++-5.3.0 CXXFLAGS="-O3 -DNDEBUG" LINKFLAGS="-whatever" ./waf configure
```

If everything goes well, you'll get an executable in `build/stcm-editor`.

Boost with C++14
----------------

[Download the latest release][boost-dl], and look at
[getting started][boost-getting-started] guide, specially the 5.2. section. If
you have boost installed globally, that can cause problems. In this case edit
`tools/build/src/engine/jambase.c` and remove/comment out the line:
```
"BOOST_BUILD_PATH = /usr/share/boost-build ;\n",
```
before running `bootstrap.sh`.

Continue until 5.2.4. You'll need to add `cxxflags=-std=c++14` to the `b2`
command line. Adding `link=static` is also a good idea to avoid dynamic loader
path problems. We currently only use the filesystem library, so you can add
`--with-filesystem` to reduce build time. I used the following command line:
```
b2 --with-filesystem toolset=gcc-5.3.0 variant=release link=static threading=single runtime-link=shared cxxflags=-std=c++14 stage
```

To actually use it, if you unpacked boost into `$boost_dir`:
```
./waf configure --boost-includes $boost_dir --boost-libs $boost_dir/stage/lib
```

(Cross) Compiling to Windows
----------------------------

Currently only clang (compiled from git, probably patched, see next section. 3.7
doesn't support exceptions properly) is supported, with MSVC 2013 lib files.
You'll also need [lld] if you want LTO or want to cross compile. I've only
tested cross compiling, but it should be possible to compile on Windows too.

Install MSVC 2013 on a Windows (virtual) machine. If you want to cross compile,
you'll need to copy directories named `include` and `lib` to your Linux box from
`Program Files (x86)/Microsoft Visual Studio 12.0/VC` and `Program Files
(x86)/Windows Kits/8.1` too (assuming default install location).

Problem #1: Linux filesystems are usually case-sensitive, but MSVC headers
pretty much expect a case-insensitive file lookup. Solution 1: store the files
on a case-insensitive fs (fat, ntfs, etc, or just mount your Windows fs).
Solution 2: use [ciopfs]. Make sure you mount `ciopfs` first, and copy into that
directory, otherwise you'll manually have to convert all files to downcase.

Problem #2: clang won't know where are your files, so you'll need some compiler
flags. For compiling you'll need: `-m32 -isystem $vc/include -isystem
$winkit/include/um -isystem $winkit/include/shared` where `$vc` and `$winkit`
refers to the folders you previously copied. Using `-isystem` prevent clang from
overflowing your terminal about how awful the microsoft headers are (we know
that). For linking, you'll need `/libpath:$vc/lib
/libpath:$winkit/lib/winv6.3/um/x86`.

To compile boost, look [here][boost-cross]. You'll need to create a
`~/user-config.jam`. Mine looks like this (clang is installed into `$clangbin`):

```
using msvc : clang : "$clangbin/clang-cl" :
  <compileflags>"-m32 -fms-compatibility-version=18 -isystem $vc/include -isystem $winkit/include/um -isystem $winkit/include/shared -Xclang -emit-llvm-bc"
  <linkflags>"/libpath:$vc/lib /libpath:$winkit/lib/winv6.3/um/x86"
  <compiler>"$clangbin/clang-cl"
  <linker>"$clangbin/lld-link"
  <setup>
  ;
```

`-Xclang -emit-llvm-bc` is used to enable LTO-ing boost. You can remove it if
you do not want it. Then use `b2 toolset=msvc-clang ...` to compile boost. You
can also create a 64-bit version, in this case remove `-m32` from cflags and
adjust the libpaths (but it's not really required if you only build static
libs).


Now you can compile this project. Use something like that:
```
CC=$clangbin/clang-cl CXX=$clangbin/clang-cl LINK_CXX=$clangbin/lld-link AR=$clangbin/llvm-lib CXXFLAGS="your cflags"  LINKFLAGS="your linkflags" ./waf configure --clang-hack --boost-includes ...
```

Some potential problems with the clang toolchain
------------------------------------------------

When building a `.dll`, `llvm-link` wants to invoke the original `lib.exe` to
create an imports lib or whatever, which won't work on Linux (and completely
unnecessary, since we export 0 symbols...).

Open `tools/lld/COFF/Driver.cpp`, find these lines:
```c++
  if (!Config->Exports.empty() || Config->DLL) {
    fixupExports();
    writeImportLibrary();
    assignExportOrdinals();
  }
```
and comment out/delete the `writeImportLibrary()` line (the others are needed
for the `dinput8.dll` hack).

The second problem is that when using LTO, llvm can fuck up the bytecode, and
fail or create an executable that'll randomly crash. See `clang.patch` for a
patch that fixes some of these problems. It'll only fixes normal 32-bit builds.
In case of 64-bit builds or the 32-bit tests, it'll enter an infinite loop
during linking... A safer alternative is to specify `/opt:lldlto=1` in the link
flags.


[boost-dl]: http://www.boost.org/users/download/
[boost-getting-started]: http://www.boost.org/doc/libs/1_60_0/more/getting_started/unix-variants.html
[boost-cross]: http://www.boost.org/build/doc/html/bbv2/tasks/crosscompile.html
[lld]: http://lld.llvm.org/
[ciopfs]: http://www.brain-dump.org/projects/ciopfs/
