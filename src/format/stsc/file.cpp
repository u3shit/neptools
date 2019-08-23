#include "file.hpp"
#include "header.hpp"
#include "../cstring_item.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"
#include "../../open.hpp"

#include <boost/algorithm/string/replace.hpp>

namespace Neptools::Stsc
{

  File::File(Source src, Flavor flavor) : flavor{flavor}
  {
    ADD_SOURCE(Parse_(src), src);
  }

  void File::Parse_(Source& src)
  {
    auto root = Create<RawItem>(src);
    SetupParseFrom(*root);
    root->Split(root->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({&*root, 0}, flavor);
  }

  void File::Inspect_(std::ostream& os, unsigned indent) const
  {
    LIBSHIT_ASSERT(GetLabels().empty());
    os << "neptools.stsc.file(neptools.stsc.flavor." << ToString(flavor) << ")";
    InspectChildren(os, indent);
  }

  static const char SEP_DASH[] = {
#define REP_MACRO(x,y,z) '-',
    BOOST_PP_REPEAT(40, REP_MACRO, )
    '\r', 0,
  };

  void File::WriteTxt_(std::ostream& os) const
  {
    for (auto& it : GetChildren())
    {
      auto str = dynamic_cast<const CStringItem*>(&it);
      if (str)
      {
        os << boost::replace_all_copy(str->string, "\\n", "\r\n")
           << "\r\n" << SEP_DASH << '\n';
      }
    }
  }

  void File::ReadTxt_(std::istream& is)
  {
    std::string line, msg;
    auto it = GetChildren().begin();
    auto end = GetChildren().end();
    while (it != end && !dynamic_cast<CStringItem*>(&*it)) ++it;

    is.exceptions(std::ios_base::badbit);
    while (!std::getline(is, line).fail())
    {
      if (line == SEP_DASH)
      {
        if (it == end)
          LIBSHIT_THROW(Libshit::DecodeError, "StscTxt: too many strings");

        LIBSHIT_ASSERT(msg.empty() || msg.substr(msg.length()-2) == "\\n");
        if (!msg.empty()) { msg.pop_back(); msg.pop_back(); }
        static_cast<CStringItem&>(*it).string = std::move(msg);

        ++it;
        while (it != end && !dynamic_cast<CStringItem*>(&*it)) ++it;

        msg.clear();
      }
      else
      {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        msg.append(line).append("\\n");
      }
    }

    if (it != end)
      LIBSHIT_THROW(Libshit::DecodeError, "StscTxt: not enough strings");
  }

  static Flavor glob_flavor = Flavor::NOIRE;
  static Libshit::Option flavor_opt{
    GetFlavorOptions(), "stsc-flavor", 1, "FLAVOR",
#define GEN_HELP(x,y) "\t\t" #x "\n"
    "Set STSC flavor:\n" NEPTOOLS_GEN_STSC_FLAVOR(GEN_HELP,),
#undef GEN_HELP
    [](auto&& args)
    {
      if (false); // NOLINT
#define GEN_IFS(x, y)                         \
      else if (strcmp(args.front(), #x) == 0) \
        glob_flavor = Flavor::x;

      NEPTOOLS_GEN_STSC_FLAVOR(GEN_IFS,)
#undef GEN_IFS
      else throw Libshit::InvalidParam{"invalid argument"};
    }};

  static OpenFactory stsc_open{[](const Source& src) -> Libshit::SmartPtr<Dumpable>
  {
    if (src.GetSize() < sizeof(HeaderItem::Header)) return nullptr;
    char buf[4];
    src.PreadGen(0, buf);
    if (memcmp(buf, "STSC", 4) == 0)
      return Libshit::MakeSmart<File>(src, glob_flavor);
    else
      return nullptr;
  }};

}

#include "file.binding.hpp"
