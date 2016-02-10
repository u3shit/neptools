#ifndef UTILS_HPP
#define UTILS_HPP
#pragma once

#include "fs.hpp"
#include <iosfwd>
#include <boost/assert.hpp>

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

// http://ficksworkshop.com/blog/14-coding/86-how-to-static-cast-std-unique-ptr
template<typename D, typename B>
std::unique_ptr<D> static_cast_ptr(std::unique_ptr<B>&& base)
{
    return std::unique_ptr<D>(static_cast<D*>(base.release()));
}

template <typename T, typename U>
T asserted_cast(U* ptr)
{
    BOOST_ASSERT(dynamic_cast<T>(ptr));
    return static_cast<T>(ptr);
}

std::ofstream OpenOut(const fs::path& pth);
std::ifstream OpenIn(const fs::path& pth);

#endif
