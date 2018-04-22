#! /bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"

src=(src/dumpable src/endian src/open src/sink src/source src/txt_serializable
     src/format/cl3 src/format/context src/format/cstring_item
     src/format/eof_item src/format/gbnl src/format/item
     src/format/primitive_item src/format/raw_item
     src/format/stcm/collection_link src/format/stcm/data
     src/format/stcm/exports src/format/stcm/file src/format/stcm/gbnl
     src/format/stcm/header src/format/stcm/instruction
     src/format/stcm/string_data
     src/format/stsc/file src/format/stsc/header src/format/stsc/instruction
     test/container/ordered_map test/container/parent_list)

. libshit/gen_binding.sh
