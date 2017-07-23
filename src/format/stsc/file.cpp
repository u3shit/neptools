#include "file.hpp"
#include "header.hpp"
#include "../cstring_item.hpp"
#include "../eof_item.hpp"
#include "../raw_item.hpp"
#include "../../open.hpp"

#include <boost/algorithm/string/replace.hpp>

namespace Neptools::Stsc
{

File::File(Source src)
{
    AddInfo(&File::Parse_, ADD_SOURCE(src), this, src);
}

void File::Parse_(Source& src)
{
    auto root = Create<RawItem>(src);
    SetupParseFrom(*root);
    root->Split(root->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({&*root, 0});
}

void File::Inspect_(std::ostream& os, unsigned indent) const
{
    NEPTOOLS_ASSERT(GetLabels().empty());
    os << "neptools.stsc.file()";
    InspectChildren(os, indent);
}

static const char SEP_DASH[] = {
#define REP_MACRO(x,y,z) char(0x81), char(0x5c),
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
                NEPTOOLS_THROW(DecodeError{"StscTxt: too many strings"});

            NEPTOOLS_ASSERT(msg.empty() || msg.substr(msg.length()-2) == "\\n");
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
        NEPTOOLS_THROW(DecodeError{"StscTxt: not enough strings"});
}

static OpenFactory stsc_open{[](Source src) -> SmartPtr<Dumpable>
{
    if (src.GetSize() < sizeof(HeaderItem::Header)) return nullptr;
    char buf[4];
    src.PreadGen(0, buf);
    if (memcmp(buf, "STSC", 4) == 0)
        return MakeSmart<File>(src);
    else
        return nullptr;
}};

}

#include "file.binding.hpp"
