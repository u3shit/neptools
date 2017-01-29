#ifndef UUID_AA1F5C12_4481_42C8_9CCD_3AD44A0D7671
#define UUID_AA1F5C12_4481_42C8_9CCD_3AD44A0D7671
#pragma once

#ifdef NEPTOOLS_BINDING_GENERATOR
#   define NEPTOOLS_META(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#   define NEPTOOLS_META(...)
#endif

#define NEPTOOLS_LUAGEN(...) NEPTOOLS_META(lua{__VA_ARGS__})
#define NEPTOOLS_NOLUA NEPTOOLS_LUAGEN(hidden=true)
#define NEPTOOLS_LUA_TEMPLATE(id, nam, ...)     \
    using id NEPTOOLS_LUAGEN(name=nam) = __VA_ARGS__

#endif
