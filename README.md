stcm-editor
===========

This tool allows you to edit dialogues in Hyperdimension Neptunia Re;Birth 3: V
Century. The file format is compatible with nr2_strool/nr3_strtool's.

Note: if you've used strtool before, you should delete the modified `.cl3` files
and reimport the `.txt`s using the original, unmodified `.cl3` files. Strtool
sometimes damages the `.cl3` files in a way that this tool can't handle.

Usage
=====

First, if you downloaded the binary Windows version, you'll need to install
[MSVC 2015 Update 1 redistributables](https://www.microsoft.com/en-us/download/details.aspx?id=49984),
if you haven't done so already (download `VC_redist.x86.exe`).

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

A note on cl3 editing and `--create-cl3`
----------------------------------------

Cl3 files have a `FILE_LINK` section that describes links between different
files. This information is currently not exported (and you can't edit them yet),
but updating an existing cl3 file will preserve them. So for the time being you
should only replace existing files in existing cl3 archives.


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
