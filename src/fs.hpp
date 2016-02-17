#ifndef UUID_4AD3415D_8161_4CDF_928B_06CB81FD6097
#define UUID_4AD3415D_8161_4CDF_928B_06CB81FD6097
#pragma once

#ifdef _MSC_VER

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#else

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#endif

#endif
