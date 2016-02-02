stcm-editor
===========

This tool allows you to edit dialogues in Hyperdimension Neptunia Re;Birth 3: V
Century. The file format is compatible with nr2_strool/nr3_strtool's.

Note: if you've used strtool before, you should delete the modified `.cl3` files
and reimport the `.txt`s using the original, unmodifed `.cl3` files. Strtool
sometimes damages the `.cl3` files in a way that this tool can't handle.

Usage
=====

The main functionality of this program is to dump out the text script inside the
`.cl3` files, and then reimport modifications. You should be able to do that by
just dragging the `.cl3` file onto the executable to extract the .txt files, and
drop the `.txt` files to import back. You can also mass-convert directories by
dropping them. By default it will export every `.cl3` which doesn't have a
corresponding `.txt`, and import where `.txt` exitst. You can override it with
the `--export-only` and `--import-only` options to only import or export.

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
