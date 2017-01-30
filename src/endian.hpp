#ifndef UUID_2B513D80_9476_4D4A_8C1B_07F8A1401F01
#define UUID_2B513D80_9476_4D4A_8C1B_07F8A1401F01
#pragma once

#include <boost/endian/arithmetic.hpp>

namespace Neptools
{

using Order = boost::endian::order;

#define NEPTOOLS_ENDIAN_TYPES(x) \
    x(Int8,   int8_t,   8)  \
    x(Int16,  int16_t,  16) \
    x(Int32,  int32_t,  32) \
    x(Int64,  int64_t,  64) \
    x(Uint8,  uint8_t,  8)  \
    x(Uint16, uint16_t, 16) \
    x(Uint32, uint32_t, 32) \
    x(Uint64, uint64_t, 64)

// use unaligned types. clang is too retarded to optimize out bitshifting...
#define NEPTOOLS_ENDIAN_TYPE_DEF(Camel, snake, bits)        \
    template <Order O>                                      \
    using Endian##Camel = boost::endian::endian_arithmetic< \
        O, snake, bits, boost::endian::align::yes>;

NEPTOOLS_ENDIAN_TYPES(NEPTOOLS_ENDIAN_TYPE_DEF)
#undef NEPTOOLS_ENDIAN_TYPE_DEF

};

#endif
