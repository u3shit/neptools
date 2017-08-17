// -*- c++ -*-
#pragma once

static constexpr const int STDOUT_FILENO = 1;
static constexpr inline bool isatty(int) noexcept { return true; }
