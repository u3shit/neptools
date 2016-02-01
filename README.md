stcm-editor
===========

This tool allows you to edit dialogues in Hyperdimension Neptunia Re;Birth 3: V
Century. The file format is compatible with nr2_strool/nr3_strtool's.

Usage
=====

Run without arguments to get a brief help. The two most useful commands are
probably `read-txt` which turns a `.cl3` file into a `.txt` file and
`write-txt`, which converts it back:

```
stcm-editor read-txt main.cl3
# edit main.cl3 using your favourite text editor
stcm-editor write-txt main.cl3.txt
```

You can also give the name of a directory instead of a single file to bach
convert all files inside it.

Use `stcm-editor --show-hidden` to display some extra commands, that are
probably useless unless you know what are you doing.


Compilation
===========

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
./waf configure --boost-includes $boost_dir --liibst-libs $boost_dir/stage/libs
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

[boost-dl]: http://www.boost.org/users/download/
[boost-getting-started]: http://www.boost.org/doc/libs/1_60_0/more/getting_started/unix-variants.html
[boost-cross]: http://www.boost.org/build/doc/html/bbv2/tasks/crosscompile.html
