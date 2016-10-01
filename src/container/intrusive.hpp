#ifndef UUID_4120A45C_2DF0_41E9_B367_956A8E211C54
#define UUID_4120A45C_2DF0_41E9_B367_956A8E211C54
#pragma once

#include <boost/intrusive/options.hpp>

namespace Neptools
{

#ifdef NDEBUG
using LinkMode = boost::intrusive::link_mode<boost::intrusive::normal_link>;
#else
using LinkMode = boost::intrusive::link_mode<boost::intrusive::safe_link>;
#endif

}

#endif
