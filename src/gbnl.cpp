#include "gbnl.hpp"
#include <map>
#include <boost/assert.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>


bool GbnlMessageDescriptor::IsValid(size_t chunk_size) const noexcept
{
    return text_offset < chunk_size;
}

bool GbnlFooter::IsValid(size_t chunk_size) const noexcept
{
    return memcmp(magic, "GBNL", 4) == 0 &&
        field_04 == 1 && field_08 == 16 && field_0c == 4 &&
        flags == 1 && descr_offset == 0 &&
        msg_descr_size == sizeof(GbnlMessageDescriptor) &&
        field_22 == 0 &&
        offset_control + sizeof(GbnlControlDescriptor) * count_control < chunk_size &&
        offset_msgs < chunk_size &&
        field_34 == 0 && field_38 == 0 && field_3c == 0;
}

Gbnl::Gbnl(const Byte* data, size_t len)
{
    if (len < sizeof(GbnlFooter))
        throw std::runtime_error("GBNL: section too short");

    auto foot = reinterpret_cast<const GbnlFooter*>(
        data + len - sizeof(GbnlFooter));
    if (!foot->IsValid(len))
        throw std::runtime_error("GBNL: invalid footer");
    field_28 = foot->field_28;
    field_30 = foot->field_30;

    auto ctrl = reinterpret_cast<const GbnlControlDescriptor*>(
        data + foot->offset_control);
    control.reserve(foot->count_control);
    for (size_t i = 0; i < foot->count_control; ++i)
        control.emplace_back(ctrl[i].first, ctrl[i].second);

    auto msgs = reinterpret_cast<const GbnlMessageDescriptor*>(
        data + foot->descr_offset);
    messages.reserve(foot->count_msgs);
    for (size_t i = 0; i < foot->count_msgs; ++i)
    {
        if (!msgs[i].IsValid(len))
            throw std::runtime_error("GBNL: invalid message descriptor");
        messages.push_back({
            msgs[i].message_id, msgs[i].lipsync_chara_id, msgs[i].field_08,
            msgs[i].field_0c, msgs[i].field_10, msgs[i].field_18,
            msgs[i].name_id, msgs[i].audio_id,
            reinterpret_cast<const char*>(data) + foot->offset_msgs + msgs[i].text_offset,
            0});
    }
    RecalcSize();

    if (GetSize() != len)
        throw std::runtime_error("GBNL: invalid size after repack");
}


static char ZEROS[16];
void Gbnl::Dump_(std::ostream& os) const
{
    GbnlMessageDescriptor msgd;

    for (const auto& m : messages)
    {
        msgd.message_id = m.message_id;
        msgd.lipsync_chara_id = m.lipsync_chara_id;
        msgd.field_08 = m.field_08;
        msgd.field_0c = m.field_0c;
        msgd.field_10 = m.field_10;
        msgd.field_18 = m.field_18;
        msgd.name_id = m.name_id;
        msgd.audio_id = m.audio_id;
        msgd.text_offset = m.offset;
        os.write(reinterpret_cast<char*>(&msgd), sizeof(GbnlMessageDescriptor));
    }
    auto msgs_end = sizeof(GbnlMessageDescriptor) * messages.size();
    auto msgs_end_round = (msgs_end + 15) / 16 * 16;
    os.write(ZEROS, msgs_end_round - msgs_end);

    GbnlControlDescriptor ctrl;
    for (const auto& c : control)
    {
        ctrl.first = c.first;
        ctrl.second = c.second;
        os.write(reinterpret_cast<char*>(&ctrl), sizeof(GbnlControlDescriptor));
    }
    auto control_end = msgs_end_round + sizeof(GbnlControlDescriptor) * control.size();
    auto control_end_round = (control_end + 15) / 16 * 16;
    os.write(ZEROS, control_end_round - control_end);

    size_t offset = 0;
    for (const auto& m : messages)
    {
        if (m.offset == offset)
        {
            os.write(m.text.c_str(), m.text.size() + 1);
            offset += m.text.size() + 1;
        }
    }
    BOOST_ASSERT(offset == msgs_size);
    auto offset_round = (offset + 15) / 16 * 16;
    os.write(ZEROS, offset_round - offset);

    GbnlFooter foot;
    memcpy(foot.magic, "GBNL", 4);
    foot.field_04 = 1;
    foot.field_08 = 16;
    foot.field_0c = 4;
    foot.flags = 1;
    foot.descr_offset = 0;
    foot.count_msgs = messages.size();
    foot.msg_descr_size = sizeof(GbnlMessageDescriptor);
    foot.count_control = control.size();
    foot.field_22 = 0;
    foot.offset_control = msgs_end_round;
    foot.field_28 = field_28;
    foot.offset_msgs = control_end_round;
    foot.field_30 = field_30;
    foot.field_34 = 0;
    foot.field_38 = 0;
    foot.field_3c = 0;
    os.write(reinterpret_cast<char*>(&foot), sizeof(GbnlFooter));
}

void Gbnl::Inspect_(std::ostream& os) const
{
    os << "gbnl(" << field_28 << ", " << field_30 << ", messages[\n";
    for (const auto& m : messages)
    {
        os << "  (" << m.message_id << ", " << m.lipsync_chara_id << ", "
           << m.field_08 << ", " << m.field_0c << ", " << m.field_10 << ", "
           << m.field_18 << ", " << m.name_id << ", "  << m.audio_id << ", ";
        DumpBytes(os, m.text);
        os << ")\n";
    }
    os << "] control[";
    for (const auto& c : control)
        os << "(" << c.first << ", " << c.second << ")";
    os << "])";
}

void Gbnl::RecalcSize()
{
    std::map<std::string, size_t> offset_map;
    size_t offset = 0;
    for (auto& m : messages)
    {
        auto x = offset_map.emplace(m.text, offset);
        if (x.second) // new item inserted
            offset += m.text.size() + 1;
        m.offset = x.first->second;
    }
    msgs_size = offset;
}

size_t Gbnl::GetSize() const noexcept
{
    size_t ret = sizeof(GbnlMessageDescriptor) * messages.size();
    ret = (ret + 15) / 16 * 16 + sizeof(GbnlControlDescriptor) * control.size();
    ret = (ret + 15) / 16 * 16 + msgs_size;
    ret = (ret + 15) / 16 * 16 + sizeof(GbnlFooter);
    return ret;
}

const char SEP_DASH[] = {
#define REP_MACRO(x,y,z) char(0x81), char(0x5c),
    BOOST_PP_REPEAT(40, REP_MACRO, )
    ' '
};

void Gbnl::WriteTxt(std::ostream& os) const
{
    for (const auto& m : messages)
    {
        os.write(SEP_DASH, sizeof(SEP_DASH));
        os << m.message_id << "\r\n"
           << boost::replace_all_copy(m.text, "\n", "\r\n") << "\r\n";
    }
    os.write(SEP_DASH, sizeof(SEP_DASH));
    os << "EOF";
}

void Gbnl::ReadTxt(std::istream& is)
{
    std::vector<Message>::iterator it{};
    std::string line, msg;

    while (is.good())
    {
        std::getline(is, line);

        if (line.compare(0, sizeof(SEP_DASH), SEP_DASH, sizeof(SEP_DASH)) == 0)
        {
            if (it != std::vector<Message>::iterator{})
            {
                boost::trim_right(msg);
                it->text = std::move(msg);
                msg.clear();
            }

            if (line.compare(sizeof(SEP_DASH), 3, "EOF") == 0)
            {
                RecalcSize();
                return;
            }
            auto id = std::strtoul(line.data() + sizeof(SEP_DASH), nullptr, 10);
            it = std::find_if(messages.begin(), messages.end(),
                              [id](const auto& m) { return m.message_id == id; });
            if (it == messages.end())
                throw std::runtime_error("GbnlTxt: invalid id in input");
        }
        else
        {
            if (it == std::vector<Message>::iterator{})
                throw std::runtime_error("GbnlTxt: data before separator");
            boost::trim_right(line);
            msg.append(line).append(1, '\n');
        }
    }
    throw std::runtime_error("GbnlTxt: EOF");
}
