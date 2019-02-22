#include "pattern.hpp"

#include <libshit/assert.hpp>

#include <boost/algorithm/searching/boyer_moore.hpp>

#define LIBSHIT_LOG_NAME "pattern"
#include <libshit/logger_helper.hpp>

namespace Neptools
{

  static bool CheckPattern(
    const Byte* ptr, const Byte* pattern, const Byte* mask, size_t len)
  {
    while (len--)
      if ((*ptr++ & *mask++) != *pattern++)
        return false;
    return true;
  }

  const Byte* Pattern::MaybeFind(Libshit::StringView data) const noexcept
  {
    size_t max_len = 0, max_i = 0;
    size_t start_i = 0;
    for (size_t i = 0; i < size; ++i)
      if (mask[i] == 0xff)
      {
        if (i - start_i + 1 > max_len)
        {
          max_len = i - start_i + 1;
          max_i = start_i;
        }
      }
      else
        start_i = i+1;

    LIBSHIT_ASSERT(max_i + max_len <= size);
    boost::algorithm::boyer_moore<const Byte*> bm{
      pattern + max_i, pattern + max_i + max_len};

    auto ptr = data.udata() + max_i;
    auto ptr_end = data.udata() + data.length() - (size - max_len - max_i);
    const Byte* res = nullptr;

    while (true)
    {
      DBG(4) << "finding " << static_cast<const void*>(ptr) << "..."
        <<  static_cast<const void*>(ptr_end) << std::endl;
      auto match = bm(ptr, ptr_end)
#if BOOST_VERSION >= 106200
        // I hope you die a horrible death
        // https://github.com/boostorg/algorithm/commit/205f5ff4bbb00ece289892089a6c94c975e97ca2
        .first
#endif
        ;
      DBG(4) << "match @" << static_cast<const void*>(match) << std::endl;
      if (match == ptr_end) break;

      if (CheckPattern(match - max_i, pattern, mask, size))
        if (res)
        {
          // todo?
          WARN << "Multiple matches for pattern "
            << static_cast<const void*>(res) << " and "
            << static_cast<const void*>(match - max_i) << std::endl;
          return nullptr;
        }
        else
          res = match - max_i;
      ptr = match + 1;
    }
    DBG(3) << "Found pattern @" << static_cast<const void*>(res) << std::endl;
    return res;
  }

}
