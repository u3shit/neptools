#ifndef FS_HPP
#define FS_HPP
#pragma once

#ifdef _MSC_VER

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#else

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#endif

#endif
