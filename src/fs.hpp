#ifndef UUID_4AD3415D_8161_4CDF_928B_06CB81FD6097
#define UUID_4AD3415D_8161_4CDF_928B_06CB81FD6097
#pragma once

#ifdef HAVE_EXPERIMENTAL_FILESYSTEM

#include <experimental/filesystem>
#include <random>

namespace fs
{
using namespace std::experimental::filesystem;

inline path unique_path()
{
    std::random_device rd;
    std::uniform_int_distribution<int> dist('a', 'z');

    char buf[16];
    for (size_t i = 0; i < 16; ++i)
        buf[i] = dist(rd);
    return path{buf, buf+16};
}
}

#else

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#endif

#endif
