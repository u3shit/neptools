STCM
====

STCM files store bytecode for a VM used by the Re;Births.

The general structure of an STCM file is:

* Header
* Padding until 0x50, `GLOBAL_DATA` string, padding until 0x70.
* Data structures
* Padding until dividable by 16, `CODE_START_\0`
* Instructions
* `EXPORT_DATA\0`
* Export entries
* `COLLECTION_LINK\0`
* Collection link header, entries until EOF

But generally the engine do not care about it, it only follows the offsets. The
strings and extra padding can be removed without breaking anything. There are
usually a lot of dead code and unused data in the STCM files.

Header
------

```c++
struct Header
{
    char magic[0x20];
    uint32_t export_offset;
    uint32_t export_count;
    uint32_t field_28; // ??
    uint32_t collection_link_offset;
};
sizeof(Header) == 0x30
```

`magic` starts with `STCM2L` (where `L` likely means little-endian), after a
null terminated describing the build date (?) of the generator follows (ignored
by the game). (Like`STCM2L Apr 22 2013 19:39:01` in R;B3).

There are `export_count` count `ExportEntry` at `export_offset` and a
`CollectionLinkHeader` at `collection_link_offset`.


Export entry
------------

```c++
struct ExportEntry
{
    uint32_t type; // 0 = CODE, 1 = DATA
    char name[0x20];
    uint32_t offset;
};
sizeof(ExportEntry) == 0x28
```

If `type == 0`, `offset` is an offset to an `InstructionHeader`, if `type == 1`,
it should be an offset to `Data` (not really used by the game). `name` is a null
terminated string.

Data
----

```c++
struct Data
{
    uint32_t type; // 0 or 1 ??
    uint32_t offset_unit;
    uint32_t field_8;
    uint32_t length;
};
sizeof(Data) = 0x10
```

`length` bytes of data follow the structure. `offset_unit` is `length/4` for
string data, 1 otherwise?

Strings are stored as null terminated strings, the length is padded to a
multiple of 4 (so a string of length 4 will have a terminating zero byte and 3
padding zero bytes at the end).


Instruction
-----------

```c++
struct InstructionHeader
{
    uint32_t is_call; // 0 or 1
    uint32_t opcode_offset;
    uint32_t param_count; // < 16
    uint32_t size;
};
sizeof(InstructionHeader) == 0x10;
```

If `is_call` is true, `opcode_offset` contains an offset to the called
instruction. If `is_call` is false, `opcode_offset` is simply an opcode number
in the VM, not an offset.

`param_count` of `Parameter` structure follows the header, followed by `size`
bytes of arbitrary payload (usually `Data` structures described above). After
the payload usually comes the next instruction. The following opcodes are known
to jump unconditionally, thus in this case the engine won't try to parse and
execute the next instruction: 0, 6.

```c++
struct Parameter
{
    uint32_t param_0;
    uint32_t param_4;
    uint32_t param_8;
};
sizeof(Parameter) == 0x0c

uint32_t TypeTag(uint32_t x) { return x >> 30; }
uint32_t Value(uint32_t x) { return x & 0x3fffffff; }

```

The meaning of the members depend on the value of `TypeTag(param_0)` (i.e. the
upper two bits of `param_0`.

### `param_0`

```c++
enum Type
{
    MEM_OFFSET = 0,
    IMMEDIATE = 1,
    INDIRECT = 2,
    SPECIAL = 3,
};```

If `TypeTag(param_0) == MEM_OFFSET`, `Value(param_0)` (the lower 30 bits of
`param_0`) contains an offset to a `Data` structure, `param_4` and `param_8`
parsed normally.

If `TypeTag(param_0) == INDIRECT`, `Value() < 256` (and not a file offset).
`param_4` must be `0x40000000`. `param_8` parsed normally.

If `TypeTag(param_0) == SPECIAL`, then there are other subcases:
```c++
enum TypeSpecial
{
    READ_STACK_MIN = 0xffffff00, // range MIN..MAX
    READ_STACK_MAX = 0xffffff0f,
    READ_4AC_MIN   = 0xffffff20, // range MIN..MAX
    READ_4AC_MAX   = 0xffffff27,
    INSTR_PTR0     = 0xffffff40,
    INSTR_PTR1     = 0xffffff41,
    COLL_LINK      = 0xffffff42,
};
```
If `(param_0 >= READ_STACK_MIN && param_0 <= READ_STACK_MAX) || (param_0 >=
READ_4AC_MIN && param_0 <= READ_4AC_MAX)`, then `param_4` and `param_8` must be
`0x40000000`.

If `param_0 == INSTR_PTR0 || param_0 == INSTR_PTR1`, then `param_4` contains an
offset to an another instruction, `param_8` must be `0x40000000`.

If `patam_0 == COLL_LINK`, `param_4` contains an offset to a `CollectionLink`
structure, `param_8` must be 8.

### `param_4` and `param_8`

The following only applies when these parameters are "parsed normally".
`param_n` refers to either `param_4` or `param_8`.

If `TypeTag(param_n) == MEM_OFFSET`, `Value(param_n)` contains an offset to ???.

If `TypeTag(param_n) == IMMEDIATE || TypeTag(param_n) == INDIRECT`,
`Value(param_n)` contains a value.

If `TypeTag(param_n) == SPECIAL`, then `(param_n >= READ_STACK_MIN && param_n <=
READ_STACK_MAX) || (param_n >= READ_4AC_MIN && param_n <= READ_4AC_MAX)`.

Collection link
---------------

```c++
struct CollectionLinkHeader
{
    uint32_t field_00;
    uint32_t offset;
    uint32_t count;
    uint32_t field_0c;
    uint32_t field_10;
    uint32_t field_14;
    uint32_t field_18;
    uint32_t field_1c;
    uint32_t field_20;
    uint32_t field_24;
    uint32_t field_28;
    uint32_t field_2c;
    uint32_t field_30;
    uint32_t field_34;
    uint32_t field_38;
    uint32_t field_3c;
};
sizeof(CollectionLinkHeader) == 0x40);
```

The `field_*` members are always zero in the files I encountered... There are
`count` `CollectionLinkEntry` structures at `offset`.


```c++
struct CollectionLinkEntry
{
    uint32_t name_0;
    uint32_t name_1;
    uint32_t field_08;
    uint32_t field_0c;
    uint32_t field_10;
    uint32_t field_14;
    uint32_t field_18;
    uint32_t field_1c;
};
sizeof(CollectionLinkEntry) == 0x20;
```

The `field_*` members are always zero in the files I encountered... `name_0` and
`name_1` are offsets to null terminated strings (they're generally after the
last entry).
