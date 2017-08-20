#include "open.hpp"

#include <libshit/except.hpp>
#include <boost/exception/errinfo_file_name.hpp>

namespace Neptools
{

  auto OpenFactory::Open(Source src) -> Libshit::NotNull<Ret>
  {
    for (auto& x : GetStore())
    {
      auto ret = x(src);
      if (ret) return MakeNotNull(ret);
    }
    LIBSHIT_THROW(Libshit::DecodeError{"Unknown input file"});
  }

  auto OpenFactory::Open(const boost::filesystem::path& fname)
    -> Libshit::NotNull<Ret>
  {
    return AddInfo(
      static_cast<Libshit::NotNull<Ret> (*)(Source)>(Open),
      [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; },
      Source::FromFile(fname.native()));
  }

}

#include "open.binding.hpp"
