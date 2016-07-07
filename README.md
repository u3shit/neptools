NepTools
========

This is a collection tools to mod in Hyperdimension Neptunia Re;Birth 3: V
Century (and probably RB 1 and 2 too) and Megadimension Neptunia VII and some
rudimentary support for Hyperdevotion Noire: Goddess Black Heart `.bin` files
(only stuff under `Main`). It contains a `.cl3` extractor/importer, a text
editor for `.cl3`/`.gstr`/`.gbin`/`.bin`, and a tool to use these files in the
game without repacking the `.pac` files in case of the Re;Births/VII (like
KitServer).

stcm-editor/cl3-tool
====================

This tool allows you to edit dialogues and some other text files. The file
format is compatible with nr2_strool/nr3_strtool's.

Note: if you've used strtool before, you should delete the modified `.cl3` files
and reimport the `.txt`s using the original, unmodified `.cl3` files. Strtool
sometimes damages the `.cl3` files in a way that this tool can't handle.

Usage
-----

The main functionality of this program is to dump out the text script inside the
`.cl3` files, and then reimport modifications. You should be able to do that by
just dragging the `.cl3` file onto the executable to extract the .txt files, and
drop the `.txt` files to import back. You can also mass-convert directories by
dropping them. By default it will export every `.cl3` which doesn't have a
corresponding `.txt`, and import where `.txt` exist. You can override it with
the `--mode export-strtool` and `--mode import-strtool` options to only import
or export.

You can also unpack and repack `.cl3` files. The easiest way to do this is to
copy/rename `stcm-editor.exe` to `cl3-tool.exe` (simply `cl3-tool` on Linux),
and drop the `.cl3` file onto the executable. It'll extract into a `.cl3.out`
directory, drop the directory onto the executable to repack. Alternatively you
can use `--mode auto-cl3`, `--mode unpack-cl3` and `--mode pack-cl3` options to
achieve this functionality without changing the filename.

Advanced usage
--------------

The tool also has an advanced mode, where you have more control over what
happens.

First you have to open a file with `--open <filename>`. Afterwards you can
inspect/modify them. Every operation is done on this file, until you open a new
one (or create an empty cl3 file with `--create-cl3`). Changes are not
automatically saved, you'll have to `--save <filename>` them. Run `stcm-editor
--help` to list all available operations.
Some examples:

    # list all files in a .cl3 file
    stcm-editor --open foo.cl3 --list-files
    # extract a .cl3 file
    stcm-editor --open foo.cl3 --extract-files output_directory
    # replace a file in .cl3
    stcm-editor --open foo.cl3 --replace-file name_in_cl3 file_name --save out.cl3
    # export txt
    stcm-editor --open foo.cl3 --export-txt foo.txt
    # chain operations together: export a file, and a txt:
    stcm-editor --open foo.cl3 --extract-file bar.tid orig.tid --export-txt foo.txt
    # chain operations: replace file and txt, extract a second cl3
    stcm-editor --open foo.cl3 --replace-file bar.tid new.tid --import-txt foo.txt --open bar.cl3 --export-files dir
    # and so on...

Server
======

This is like KitServer, except it's open source and modifies some internal game
functions instead of creating virtual files that look like the original `.pac`
files. The main differences between KitServer and NepTools' server:

* It also supports a launcher-less method, see usage below.
* No tid_tool integration (for now). You have to convert your pngs to tid if you
  want to use them.
* Integrated stcm-editor: just drop the `.cl3.txt` (and `.gbin.txt` and
  `.gstr.txt`) files into the corresponding directory, it'll import them on the
  fly. *Note*: if there's an import error it'll silently fall back to the
  builtin file. Get the debug version if you want to see the error message or
  try running `stcm-editor` on the file.
* Use `neptools` instead of `KitFolder`, remove the last 5 digits from folder
  names inside `data`.
* Also has some kind of debug console, download a debug release and see usage
  notes below.
* Very early release version: anything may change at any time, nothing is
  guaranteed to work...

Usage
-----

There are two ways to use it. The first one mirrors the original KitServer:
extract `launcher.exe` and `neptools-server.dll` into the game directory. In
this case you'll have to use launcher.exe to launch the game, if you simply
launch `NeptuniaReBirth[123].exe`/`NeptuniaVII.exe`, Neptools won't be loaded,
and you'll run an unmodded game.

The second way is to extract only `neptools-server.dll` and rename it to
`dinput8.dll`. In this case there's no launcher, simply start the original
executable. In case of RB3, it may not work this way. The problem is that the
directory name steam chooses (`Hyperdimension Neptunia Re;Birth3`) confuses the
Windows DLL loader, and will treat `;` as a path separator. If you do not want
to rename the directory (which would upset steam), you'll have to place
`dinput8.dll` into a subdirectory named `Birth3` (so it should end up in
`steamapps\common\Hyperdimension Neptunia Re;Birth3\Birth3\dinput8.dll`).
(If you're an Arfoire/Magiquone supporter and installed RB1/2 into somethin like
`whatever\Neptunia Re;Birth2`, you'll have to put it into `Birth2`. Basically
create a directory named whatever after the semicolon is.)

*Note for Linux/Wine users*: wine by default gives priority to it's builtin
version of `dinput8.dll`, ignoring this custom version. To fix it run `winecfg`,
select Libraries tab, write `dinput8` under New override for library, click Add,
make sure it says (native, builtin). Also you'll probably need MSVC2013
runtimes, if you didn't already install it for the game (`winetricks
vcrun2013`).

To temporarily disable modding in this case, you'll either have to temporarly
delete/rename `dinput8.dll` or start `NeptuniaReBirth?.exe` with the `--disable`
parameter (this is not exactly same as removing the dll as Neptools will be
still loaded, but it will try to minimize changes made to the game...)

In either case, you'll have to create a directory named `neptools`, and place
files to be replaced inside it. If you want to replace (for example)
`database/stitem.gbin` inside `data/SYSTEM00000`, place it into
`neptools/data/SYSTEM/database/stitem.gbin` (that five zeros you normally get is
actually an artifact of the current `.cpk`/`.pac` extractor doesn't handle the
format correctly, I will rant more about it someday...)

It also has some command line flags, use `launcher.exe --help` or
`NeptuniaReBirth?.exe --help` to get a list. If you have a debug build, you can
use `--console -lcpk=2` (or `--log-to-file=filename -lcpk=2` to save it to a
file) to list all files opened by the game (it'll cause significant slowdown
though, especially with the console method).

Compilation
===========

See [COMPILE.md](COMPILE.md) if you downloaded a source distribution. Otherwise
refer to http://github.com/u3shit/stcm-editor.

License
=======

This program is free software. It comes without any warranty, to the extent
permitted by applicable law. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2, as
published by Sam Hocevar. See http://www.wtfpl.net/ for more details.
