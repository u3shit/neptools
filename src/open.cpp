#include "open.hpp"

#include <libshit/except.hpp>

namespace Neptools
{

  auto OpenFactory::Open(Source src) -> Libshit::NotNull<Ret>
  {
    for (auto& x : GetStore())
    {
      auto ret = x(src);
      if (ret) return MakeNotNull(ret);
    }
    LIBSHIT_THROW(Libshit::DecodeError, "Unknown input file");
  }

  auto OpenFactory::Open(const boost::filesystem::path& fname)
    -> Libshit::NotNull<Ret>
  {
    LIBSHIT_ADD_INFOS(
      return Open(Source::FromFile(fname.native())),
      "File name", fname.string());
  }

}

#include "open.binding.hpp"
