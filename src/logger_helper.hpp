// Do not include this header in public headers!
// #define NEPTOOLS_LOG_NAME "module_name"
// before including this file

#ifndef UUID_C94AAE56_E97F_45DC_A3AB_5475B7C3BDCD
#define UUID_C94AAE56_E97F_45DC_A3AB_5475B7C3BDCD
#pragma once

#include "logger.hpp"

#define LOG(level) NEPTOOLS_LOG(NEPTOOLS_LOG_NAME, level)
#define ERR  NEPTOOLS_ERR(NEPTOOLS_LOG_NAME)
#define WARN NEPTOOLS_WARN(NEPTOOLS_LOG_NAME)
#define INFO NEPTOOLS_INFO(NEPTOOLS_LOG_NAME)
#define DBG(level) NEPTOOLS_DBG(NEPTOOLS_LOG_NAME, level)

#endif
