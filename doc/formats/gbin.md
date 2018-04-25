GBIN and GSTR
=============

GBIN (stored inside `.gbin` files in SYSTEM/database, and inside `.cl3`s) and
GSTR (stored in `.gstr` files in SYSTEM/database) are quite similar. All
integers are little-endian.

The sections described below (Header/Footer, Type descriptor, Struct and String
table) are aligned to 16 bytes in GBIN, but unaligned in GSTR...

Header/Footer
-------------

```c++
struct HeaderFooter
{
    char magic[3];
    char endian;
    uint16_t field_04; // always 1
    uint16_t field_06; // always 0
    uint32_t field_08; // always 16
    uint32_t field_0c; // always 4
    uint32_t flags;
    uint32_t struct_offset;
    uint32_t struct_count;
    uint32_t struct_size;
    uint32_t types_count;
    uint32_t types_offset;
    uint32_t field_28; // ??
    uint32_t string_offset;
    uint32_t field_30; // 0 or 4
    uint32_t padding[3]; // always 0
};
sizeof(HeaderFooter) == 0x40
```

In GSTR the above struct is a header, at the beginning of the file, magic string
is `GST`. In GBIN it's a footer (so it starts at `EOF-sizeof(HeaderFooter)`),
and the magic is `GBN`... Endian is denoted by `endian`, if `L` means
little-endian and `B` means big-endian.

`flags` is 1 if there's any string stored in the file (see Type descriptor
below), 0 otherwise. `struct_offset` is 0 in GBIN and 0x40 in GSTR files (but
other values should work too). Offsets are relative to the beginning of the
file.

Type descriptor
---------------

This file contains a list of some structure. However the structure is actually
defined inside the file, so you can figure out the types of each fields.

```c++
struct TypeDescriptor
{
    uint16_t type;
    uint16_t offset;
};
sizeof(TypeDescriptor) == 4;
```

They start at `header.types_offset`, and there's `header.types_count` of them.
They're sorted into ascending order by `offset`. Each entry describes a field
inside the struct, `offset` is an offset from the beginning of the struct. The
valid values for `type` are:

```c++
enum Type
{
    UINT32 = 0,
    UINT8  = 1,
    UINT16 = 2,
    FLOAT  = 3,
    STRING = 5,
};
```

`UINT16`, and `UINT32` are standard 16 bit and 32 bit integers (not sure if
they're signed or unsigned though, probably the format makes no difference
between them), `FLOAT` is a standard 32 bit float (as used by x86 CPUs).

`UINT8` is a bit special: it's normally a single byte, but can also mean a fixed
length zero terminated string. They both end up as `type=1`. This editor
currently uses a heuristic: if the offset of the next entry is this entry's
offset + 1 (taking aligning into account), it's probably a single byte,
otherwise a fixed length string.

`STRING` is a 32 bit integer, an offset into the string table. Sometimes it'll
be -1 (0xffffffff), that's an invalid string (maybe NULL pointer originally?).

The fields are aligned: `UINT32`, `FLOAT` and `STRING` are aligned to 4 bytes
and `UINT16` to 2 bytes.

Struct
------

The structs start at `header.struct_offset` and there are `header.struct_count`
of them. Every struct is `header.struct_size` bytes length, the fields are
described by the type descriptor.


String table
------------

This is optional, it only exists if `header.flags == 1`. It starts at
`header.string_offset`. Each contains a bunch of zero terminated strings
sequentially, and referenced inside `STRING` type fields.

There's an optimization in the original files: if the same string appears
multiple times, it's only stored once in the table, the identical `STRING`
fields will get the same offset.
