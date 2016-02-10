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

Cross compiling to Windows
--------------------------

Get a cross compiler. Refer to your distro's documentation or google. For boost,
look [here][boost-cross]. You'll have to specify your cross compiler when
configuring:
```
CXX=i686-w64-mingw32-g++-5.3.0 LINKFLAGS="-static-libstdc++ -static-libgcc" ./waf configure ...
```

Specifying the `-static-*` options are not required, but in that case you'll
manually have to copy the required dlls next to the executable.

Compiling with MSVC
-------------------

Now there is some experimental support to compile stcm-editor with MSVC 2015.
Note that like everything else, I've only tested it under Linux...

You mostly want to follow [msvconwine] and [msvconwine-bug] to get a usable MSVC
compiler under Linux. I've used Visual Studio Community 2015 with Update 1. You
can comment out `DIA SDK` and `MFC` lines from `package_from_installed.py`,
they're not needed. Run the script like:
```
python package_from_installed.py -w 10.0.10240.0 2015
```
It'll create a zip file. Move that to your Linux box and unzip it somewhere.
You'll also need `C:\Program Files (x86)\Windows Kits\8.1\winv6.3`, copy it
inside `win_sdk/Lib` where you unpacked the zip. You can trash the Windows VM
now if you want.

Onto Linux now. You'll need a recent-ish wine and probably `winetricks
vcrun2015`. Also since setting DLL paths under wine is a complete mess (but they
call it a feature), you probably better run `cp sys32/*.dll VC/bin` where you
extracted the zip.

I've created a simple script file here:
```
#! /bin/bash

# http://stackoverflow.com/a/246128
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

EXEC="$(basename "$0")"
if [[ $EXEC = run_cmd ]]; then
    EXEC="$1"
    shift
fi

export INCLUDE="$INCLUDE;$DIR/VC/include;$DIR/win_sdk/Include/10.0.10240.0/ucrt"
export LIB="$LIB;$DIR/VC/lib;$DIR/win_sdk/Lib/winv6.3/um/x86;$DIR/win_sdk/Lib/10.0.10240.0/ucrt/x86"
export WINEDLLOVERRIDES="MSVCP140,vcruntime140,ucrtbase,api-ms-win-crt-locale-l1-1-0,api-ms-win-crt-runtime-l1-1-0,api-ms-win-crt-stdio-l1-1-0,api-ms-win-crt-string-l1-1-0,api-ms-win-crt-heap-l1-1-0,api-ms-win-crt-conio-l1-1-0=n"
export WINEDEBUG=-all

exec wine "$DIR/VC/bin/$EXEC.exe" "$@"
```

Place it in the folder where you extracted the zip file as `run_cmd` and make it
executable. Create symlinks to this file as `cl`, `link`, `lib` somewhere in
your `$PATH`.

Reality check: the following should work:
```
echo -n '#include <iostream>\nint main() { std::cout << "Hello" << std::endl; return 0; }' > foo.cpp
cl /MD /EHsc foo.cpp
wine foo.exe
```

Now onto stcm-editor: you do not need to compile anything with boost, as it uses
the built-in filesystems library instead of boost's one (you'll still need the
header though). To configure the project:
```
CXX=cl LINK_CXX=link AR=lib ./waf configure --msvc-hack --boost-includes $boost_dir ...
```

If checking for cl and stuff takes ages (and zero CPU usage) it because of
`wineserver`: it's automatically started by wine when you start a program, and
shuts down 3 seconds after the last Windows process quits. Since `waf` will wait
`wineserver` termination if it's started automatically like this, you better
start one manually before: `wineserver -p`. The `-p` flags prevents the
automatic shutdown.

During linking you'll probably get an ugly `Unhandled Exception` message with
some managed shit in the backtrace. You can probably(?) ignore this message as
it still creates a working executable...


[boost-dl]: http://www.boost.org/users/download/
[boost-getting-started]: http://www.boost.org/doc/libs/1_60_0/more/getting_started/unix-variants.html
[boost-cross]: http://www.boost.org/build/doc/html/bbv2/tasks/crosscompile.html
[msvconwine]: https://github.com/ehsan/msvc2013onwine
[msvconwine-bug]: https://github.com/ehsan/msvc2013onwine/issues/2
