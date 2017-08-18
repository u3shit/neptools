#ifndef UUID_AA1F5C12_4481_42C8_9CCD_3AD44A0D7671
#define UUID_AA1F5C12_4481_42C8_9CCD_3AD44A0D7671
#pragma once

#ifdef LIBSHIT_BINDING_GENERATOR
#  define LIBSHIT_META(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#  define LIBSHIT_META(...)
#endif

#define LIBSHIT_LUAGEN(...) LIBSHIT_META(lua{__VA_ARGS__})
#define LIBSHIT_NOLUA LIBSHIT_LUAGEN(hidden=true)
#define LIBSHIT_LUA_TEMPLATE(id, luagen, ...) \
  using id LIBSHIT_LUAGEN luagen = __VA_ARGS__

#endif
