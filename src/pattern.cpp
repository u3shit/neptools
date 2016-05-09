#include "pattern.hpp"
#include <boost/algorithm/searching/boyer_moore.hpp>

#define NEPTOOLS_LOG_NAME "pattern"
#include "logger_helper.hpp"

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

const Byte* Pattern::MaybeFind(const Byte* data, size_t data_length) const noexcept
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

    NEPTOOLS_ASSERT(max_i + max_len <= size);
    boost::algorithm::boyer_moore<const Byte*> bm{
        pattern + max_i, pattern + max_i + max_len};

    auto ptr = data + max_i;
    auto ptr_end = data + data_length - (size - max_len - max_i);
    const Byte* res = nullptr;

    while (true)
    {
        auto match = bm(ptr, ptr_end);
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
