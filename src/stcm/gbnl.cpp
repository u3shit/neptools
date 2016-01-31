#include "gbnl.hpp"
#include "../context.hpp"
#include <iostream>

namespace Stcm
{

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

GbnlItem::GbnlItem(Key k, Context* ctx, const Byte* data, size_t len)
    : Item{k, ctx}
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
            reinterpret_cast<const char*>(data) + foot->offset_msgs + msgs[i].text_offset});
    }
}

GbnlItem* GbnlItem::CreateAndInsert(RawItem* ritem)
{
    auto nitem = ritem->GetContext()->Create<GbnlItem>(
        ritem->GetPtr(), ritem->GetSize());
    auto ret = nitem.get();
    ritem->Replace(std::move(nitem));
    return ret;
}

static char ZEROS[16];
void GbnlItem::Dump(std::ostream& os) const
{
    GbnlMessageDescriptor msgd;
    msgd.text_offset = 0;

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
        os.write(reinterpret_cast<char*>(&msgd), sizeof(GbnlMessageDescriptor));
        msgd.text_offset += m.text.size() + 1;
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

    auto data_end = control_end_round;
    for (const auto& m : messages)
    {
        os.write(m.text.c_str(), m.text.size() + 1);
        data_end += m.text.size() + 1;
    }
    auto data_end_round = (data_end + 15) / 16 * 16;
    os.write(ZEROS, data_end_round - data_end);

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

void GbnlItem::PrettyPrint(std::ostream& os) const
{
    os << "gbnl(" << field_28 << ", " << field_30 << ", messages[\n";
    for (const auto& m : messages)
        os << "  (" << m.message_id << ", " << m.lipsync_chara_id << ", "
           << m.field_08 << ", " << m.field_0c << ", " << m.field_10 << ", "
           << m.field_18 << ", " << m.name_id << ", "  << m.audio_id << ", "
           << m.text << ")\n";
    os << "] control[";
    for (const auto& c : control)
        os << "(" << c.first << ", " << c.second << ")";
    os << "])";
}

size_t GbnlItem::GetSize() const noexcept
{
    size_t ret = sizeof(GbnlMessageDescriptor) * messages.size();
    ret = (ret + 15) / 16 * 16 + sizeof(GbnlControlDescriptor) * control.size();
    ret = (ret + 15) / 16 * 16;
    for (const auto& m : messages)
        ret += m.text.size() + 1;
    ret = (ret + 15) / 16 * 16 + sizeof(GbnlFooter);
    return ret;
}

}
