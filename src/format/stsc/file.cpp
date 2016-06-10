#include "file.hpp"
#include "header.hpp"
#include "string.hpp"
#include "../raw_item.hpp"
#include "../eof_item.hpp"

#include <boost/algorithm/string/replace.hpp>

namespace Neptools
{
namespace Stsc
{

File::File(Source src)
{
    AddInfo(&File::Parse_, ADD_SOURCE(src), this, src);
}

void File::Parse_(Source& src)
{
    auto root = Create<RawItem>(src);
    auto root_sav = root.get();
    SetRoot(std::move(root));
    root_sav->Split(root_sav->GetSize(), Create<EofItem>());
    HeaderItem::CreateAndInsert({root_sav, 0});
}

static const char SEP_DASH[] = {
#define REP_MACRO(x,y,z) char(0x81), char(0x5c),
    BOOST_PP_REPEAT(40, REP_MACRO, )
    '\r', 0,
};

void File::WriteTxt_(std::ostream& os) const
{
    for (auto it = GetRoot(); it; it = it->GetNext())
    {
        auto str = dynamic_cast<const StringItem*>(it);
        if (str)
        {
            os << boost::replace_all_copy(str->str, "\\n", "\r\n")
               << "\r\n" << SEP_DASH << '\n';
        }
    }
}

void File::ReadTxt_(std::istream& is)
{
    std::string line, msg;
    auto it = GetRoot();
    while (it && !dynamic_cast<StringItem*>(it)) it = it->GetNext();

    is.exceptions(std::ios_base::badbit);
    while (!std::getline(is, line).fail())
    {
        if (line == SEP_DASH)
        {
            if (!it)
                NEPTOOLS_THROW(DecodeError{"StscTxt: too many strings"});

            NEPTOOLS_ASSERT(msg.empty() || msg.substr(msg.length()-2) == "\\n");
            if (!msg.empty()) { msg.pop_back(); msg.pop_back(); }
            static_cast<StringItem*>(it)->str = std::move(msg);

            it = it->GetNext();
            while (it && !dynamic_cast<StringItem*>(it)) it = it->GetNext();

            msg.clear();
        }
        else
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            msg.append(line).append("\\n");
        }
    }

    if (it)
        NEPTOOLS_THROW(DecodeError{"StscTxt: not enough strings"});
}

}
}
