CL3
===

CL3 is an archive format, that contains uncompressed files plus links between
them. All integers are little-endian. Offsets are 0x40 bytes aligned in the
original files (this is not a requirement).

Header
------

```c++
struct Header
{
    char magic[3];     // always "CL3"
    char endian;
    uint32_t field_04; // always 0
    uint32_t field_08; // always 3
    uint32_t sections_count;
    uint32_t sections_offset;
    uint32_t field_14;
    // maybe more?
};
sizeof(Header) == 0x18
```

`endian` is `L` if the file is little-endian, `B` if big-endian. `field_14` is
either 0, 1 or 2 (at least in RB3). Script `.cl3`s use 1, effect `.cl3`s use 2.

Sections
--------

Starting at `sections_offset` (relative to file beginning), there are
`sections_count` sections.

```c++
struct Section
{
    char name[0x20];     // '\0' terminated
    uint32_t count;
    uint32_t data_size;
    uint32_t data_offset;
    uint32_t padding[9]; // always 0
};
sizeof(Section) == 0x50
```

There are two known sections: `FILE_COLLECTION` and `FILE_LINK`. Each has a
payload of `data_size` bytes starting at `data_offset`.

FILE_COLLECTION
---------------

This is used to store the actual files. It begins with `count` entries:
```c++
struct FileEntry
{
    char name[0x200];     // '\0' terminated
    uint32_t field_200;
    uint32_t data_offset;
    uint32_t data_size;
    uint32_t link_start;
    uint32_t link_count;
    uint32_t padding[7];  // always 0
};
sizeof(FileEntry) == 0x230
```

`Section::data_size` includes both the size of these entries and the actual file
data size (it's `last_entry.offset + last_entry.size + pad`).

`name` is the name of the file. `field_200` is either 0 for all files, or they
contain the index of the file (?). The file contents start at `data_offset`,
(unlike other offsets) **relative to the beginning of FILE_COLLECTION**, and
it's `data_size` bytes long. `link_count` contains the number of links this file
has (see below), `link_start` is an index inside the `FILE_LINK`.

FILE_LINK
---------

It contains `count` entries:
```c++
struct LinkEntry
{
    uint32_t field_00;   // always 0
    uint32_t linked_file_id;
    uint32_t link_id;
    uint32_t padding[5]; // always 0
};
sizeof(LinkEntry) == 0x20;
```

`Section::data_size` is `count*sizeof(LinkEntry)`. `linked_file_id` contains the
index of the link destination (inside `FILE_COLLECTION`). `link_id` counts the
index of the current file's link (it's an incrementing counter reset with every
new file).

In the original files, files and the corresponding link entries are in the same
order, if a file has 0 links, the `link_start` is still set to where it would
began if it'd actually have links.

Example
-------

Probably the way I described this whole link thing is not clear enough, so
here's an example:

`FILE_COLLECTION` contains this:
```c
  [0] = { .name = "a.bin", .link_start = 0, .link_count = 2 },
  [1] = { .name = "b.bin", .link_start = 2, .link_count = 1 },
  [2] = { .name = "c.bin", .link_start = 3, .link_count = 0 },
  [3] = { .name = "d.bin", .link_start = 3, .link_count = 1 },
```

And `FILE_LINK` contains this:
```c
  [0] = { .linked_file_id = 1, link_id = 0 },
  [1] = { .linked_file_id = 2, link_id = 1 },
  [2] = { .linked_file_id = 2, link_id = 0 },
  [3] = { .linked_file_id = 1, link_id = 0 },
```

This means that `a.bin` has two links, starting at 0 in `FILE_LINK`. `link[0]`
says it's a link to `file[1]` (i.e. `b.bin`), and `link[1]` says it's a link to
`file[2]` (i.e. `c.bin`).

`b.bin` only has one link, at `link[2]`. It's a link to `file[2]` (i.e.
`c.bin`).

`c.bin` has zero links, but `link_start` still contains 3, because the links
would be there if there were one. Finally `d.bin` contains a single link, still
starting at 3, since `3+0=0`.

The actual link graph looks like this:
```
a.bin ---> b.bin <--- d.bin
  |          |
  |          v
  +------> c.bin
```
