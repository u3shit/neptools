#ifndef UUID_4120A45C_2DF0_41E9_B367_956A8E211C54
#define UUID_4120A45C_2DF0_41E9_B367_956A8E211C54
#pragma once

#include <libshit/except.hpp>
#include <boost/intrusive/options.hpp>

namespace Neptools
{

#ifdef NDEBUG
  using LinkMode = boost::intrusive::link_mode<boost::intrusive::normal_link>;
#else
  using LinkMode = boost::intrusive::link_mode<boost::intrusive::safe_link>;
#endif

  LIBSHIT_GEN_EXCEPTION_TYPE(ContainerConsistency, std::logic_error);
  // trying to add an already linked item to an intrusive container
  LIBSHIT_GEN_EXCEPTION_TYPE(ItemAlreadyAdded, ContainerConsistency);
  // item not linked, linked to a different container, item is a root node, ...
  LIBSHIT_GEN_EXCEPTION_TYPE(ItemNotInContainer, ContainerConsistency);

}

#endif
