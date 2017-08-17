// Do not include this header in public headers!
// #define LIBSHIT_LOG_NAME "module_name"
// before including this file

#ifndef UUID_C94AAE56_E97F_45DC_A3AB_5475B7C3BDCD
#define UUID_C94AAE56_E97F_45DC_A3AB_5475B7C3BDCD
#pragma once

#include "logger.hpp"

#define LOG(level) LIBSHIT_LOG(LIBSHIT_LOG_NAME, level)
#define ERR  LIBSHIT_ERR(LIBSHIT_LOG_NAME)
#define WARN LIBSHIT_WARN(LIBSHIT_LOG_NAME)
#define INFO LIBSHIT_INFO(LIBSHIT_LOG_NAME)
#define DBG(level) LIBSHIT_DBG(LIBSHIT_LOG_NAME, level)

#endif
