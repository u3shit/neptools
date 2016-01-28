#ifndef UTILS_HPP
#define UTILS_HPP
#pragma once

template <typename T, typename P>
void EraseIf(T& cnt, P fun)
{
    for (auto it = cnt.begin(); it != cnt.end(); )
    {
        if (fun(*it))
            it = cnt.erase(it);
        else
            ++it;
    }
}

template <typename T>
void MaybeRehash(T& map) noexcept
{
    if (map.load_factor() < map.max_load_factor() / 2)
        try
        {
            map.rehash(0);
        }
        catch (...)
        {
            // uh-oh
        }
}

#endif
