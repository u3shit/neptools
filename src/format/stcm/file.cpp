#include "file.hpp"
#include "gbnl.hpp"
#include "header.hpp"
#include "../eof_item.hpp"
#include "../item.hpp"
#include "../../open.hpp"

namespace Neptools::Stcm
{

  File::File(Source src)
  {
    ADD_SOURCE(Parse_(src), src);
  }

  void File::Parse_(Source& src)
  {
    auto root = Create<RawItem>(src);
    SetupParseFrom(*root);
    root->Split(root->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({root.get(), 0});
  }

  void File::Inspect_(std::ostream& os, unsigned indent) const
  {
    LIBSHIT_ASSERT(GetLabels().empty());
    os << "neptools.stcm.file()";
    InspectChildren(os, indent);
  }

  void File::SetGbnl(GbnlItem& gbnl) noexcept
  {
    LIBSHIT_ASSERT(&gbnl.GetUnsafeContext() == this);
    if (!first_gbnl) first_gbnl = &gbnl;
  }

  void File::Gc() noexcept
  {
    for (auto it = GetChildren().begin(); it != GetChildren().end(); )
      if (dynamic_cast<RawItem*>(&*it) && it->GetLabels().empty())
        it = GetChildren().erase(it);
      else
        ++it;
  }

  void File::WriteTxt_(std::ostream& os) const
  { if (first_gbnl) first_gbnl->WriteTxt(os); }

  void File::ReadTxt_(std::istream& is)
  { if (first_gbnl) first_gbnl->ReadTxt(is);  }

  static OpenFactory stcm_open{[](const Source& src) -> Libshit::SmartPtr<Dumpable>
  {
    if (src.GetSize() < sizeof(HeaderItem::Header)) return nullptr;
    char buf[4];
    src.PreadGen(0, buf);
    if (memcmp(buf, "STCM", 4) == 0)
      return Libshit::MakeSmart<File>(src);
    else
      return nullptr;
  }};

}

#include <libshit/lua/table_ret_wrap.hpp>
#include "file.binding.hpp"
