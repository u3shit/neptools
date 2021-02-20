#ifndef UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#define UUID_741BE2A9_1F86_4DD5_A032_0D7D5CCBA762
#pragma once

#include "utils.hpp"

#include <libshit/except.hpp>

#include <string_view>

namespace Neptools
{

  struct Pattern
  {
    const Byte* pattern;
    const Byte* mask;
    size_t size;

    const Byte* MaybeFind(std::string_view data) const noexcept;

    const Byte* Find(std::string_view data) const
    {
      auto ret = MaybeFind(data);
      if (!ret) LIBSHIT_THROW(std::runtime_error, "Couldn't find pattern");
      return ret;
    }
  };

}
#endif
