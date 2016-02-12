#include "gbnl.hpp"
#include <map>
#include <boost/assert.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <iostream>

bool GbnlFooter::IsValid(size_t chunk_size) const noexcept
{
    return memcmp(magic, "GBNL", 4) == 0 &&
        field_04 == 1 && field_08 == 16 && field_0c == 4 &&
        flags == 1 && descr_offset == 0 &&
        descr_offset + msg_descr_size * count_msgs < chunk_size &&
        field_22 == 0 &&
        offset_types + sizeof(GbnlTypeDescriptor) * count_types < chunk_size &&
        offset_msgs < chunk_size &&
        field_34 == 0 && field_38 == 0 && field_3c == 0;
}

static void SetUsed(std::vector<bool>& used, size_t offset, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        if (used[offset+i])
            throw std::runtime_error{"GBNL: overlapping values"};
        used[offset+i] = true;
    }
}

Gbnl::Gbnl(const Byte* data, size_t len)
{
    if (len < sizeof(GbnlFooter))
        throw std::runtime_error{"GBNL: section too short"};

    auto foot = reinterpret_cast<const GbnlFooter*>(
        data + len - sizeof(GbnlFooter));
    if (!foot->IsValid(len))
        throw std::runtime_error{"GBNL: invalid footer"};
    field_28 = foot->field_28;
    field_30 = foot->field_30;

    auto types = reinterpret_cast<const GbnlTypeDescriptor*>(
        data + foot->offset_types);
    msg_descr_size = foot->msg_descr_size;
    std::vector<bool> used_bytes;
    used_bytes.resize(msg_descr_size);

    DynamicStruct::StructTypeBuilder bld;
    for (size_t i = 0; i < foot->count_types; ++i)
        switch (types[i].type)
        {
        case GbnlTypeDescriptor::UINT_8:
            SetUsed(used_bytes, types[i].offset, 1);
            bld.Add<uint8_t>();
            break;
        case GbnlTypeDescriptor::UINT_16:
            SetUsed(used_bytes, types[i].offset, 2);
            bld.Add<uint16_t>();
            break;
        case GbnlTypeDescriptor::UINT_32:
            SetUsed(used_bytes, types[i].offset, 4);
            bld.Add<uint32_t>();
            break;
        case GbnlTypeDescriptor::FLOAT:
            SetUsed(used_bytes, types[i].offset, 4);
            bld.Add<float>();
            break;
        case GbnlTypeDescriptor::STRING:
            SetUsed(used_bytes, types[i].offset, 4);
            bld.Add<OffsetString>();
            break;
        default:
            throw std::runtime_error{"GBNL: invalid type"};
        }
    if (std::find(used_bytes.begin(), used_bytes.end(), false) != used_bytes.end())
        throw std::runtime_error{"GBNL: hole in values"};

    type = bld.Build();

    auto msgs = data + foot->descr_offset;
    messages.reserve(foot->count_msgs);
    for (size_t i = 0; i < foot->count_msgs; ++i)
    {
        messages.emplace_back(type);
        auto& m = messages.back();
        for (size_t i = 0; i < foot->count_types; ++i)
        {
            auto ptr = msgs + types[i].offset;
            switch (types[i].type)
            {
            case GbnlTypeDescriptor::UINT_8:
                m.Get<uint8_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint8_t*>(ptr);
                break;
            case GbnlTypeDescriptor::UINT_16:
                m.Get<uint16_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint16_t*>(ptr);
                break;
            case GbnlTypeDescriptor::UINT_32:
                m.Get<uint32_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                break;
            case GbnlTypeDescriptor::FLOAT:
            {
                uint32_t x = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                m.Get<float>(i) = *reinterpret_cast<float*>(&x);
                break;
            }
            case GbnlTypeDescriptor::STRING:
                uint32_t offs = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                if (offs > len - foot->offset_msgs)
                    throw std::runtime_error{"GBNL: string offset too big"};
                auto str = reinterpret_cast<const char*>(data + foot->offset_msgs + offs);

                m.Get<OffsetString>(i) = {str, 0};
                break;
            }
        }

        msgs += msg_descr_size;
    }
    RecalcSize();

    if (msg_descr_size != foot->msg_descr_size || GetSize() != len)
        throw std::runtime_error("GBNL: invalid size after repack");
}

namespace
{
struct WriteDescr
{
    char* ptr;
    void operator()(uint8_t x)
    {
        *reinterpret_cast<boost::endian::little_uint8_t*>(ptr) = x;
        ptr += 1;
    }
    void operator()(uint16_t x)
    {
        *reinterpret_cast<boost::endian::little_uint16_t*>(ptr) = x;
        ptr += 2;
    }
    void operator()(uint32_t x)
    {
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = x;
        ptr += 4;
    }
    void operator()(float y)
    {
        uint32_t x = *reinterpret_cast<uint32_t*>(&y);
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = x;
        ptr += 4;
    }
    void operator()(const Gbnl::OffsetString& os)
    {
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = os.offset;
        ptr += 4;
    }
};

struct GetType
{
    uint16_t operator()(uint8_t*)  { return GbnlTypeDescriptor::UINT_8; }
    uint16_t operator()(uint16_t*) { return GbnlTypeDescriptor::UINT_16; }
    uint16_t operator()(uint32_t*) { return GbnlTypeDescriptor::UINT_32; }
    uint16_t operator()(float*)    { return GbnlTypeDescriptor::FLOAT; }
    uint16_t operator()(Gbnl::OffsetString*) { return GbnlTypeDescriptor::STRING; }
};

struct GetSize
{
    size_t operator()(Gbnl::OffsetString*) { return 4; }
    template <typename T> size_t operator()(T*) { return sizeof(T); }
};

struct SumSize
{
    size_t len = 0;
    template <typename T> void operator()(T* x) { len += GetSize{}(x); }
};
}

static char ZEROS[16];
void Gbnl::Dump_(std::ostream& os) const
{
    char msgd[msg_descr_size] = {0};
    for (const auto& m : messages)
    {
        m.ForEach<uint8_t, uint16_t, uint32_t, OffsetString>(WriteDescr{msgd});
        os.write(msgd, msg_descr_size);
    }

    auto msgs_end = msg_descr_size * messages.size();
    auto msgs_end_round = (msgs_end + 15) / 16 * 16;
    os.write(ZEROS, msgs_end_round - msgs_end);

    GbnlTypeDescriptor ctrl;
    uint16_t offs = 0;
    for (size_t i = 0; i < type->item_count; ++i)
    {
        ctrl.type = type->Visit<size_t, uint8_t, uint16_t, uint32_t, float, OffsetString>(i, GetType{});
        ctrl.offset = offs;
        offs += type->Visit<size_t, uint8_t, uint16_t, uint32_t, float, OffsetString>(i, ::GetSize{});
        os.write(reinterpret_cast<char*>(&ctrl), sizeof(GbnlTypeDescriptor));
    }
    auto control_end = msgs_end_round + sizeof(GbnlTypeDescriptor) * type->item_count;
    auto control_end_round = (control_end + 15) / 16 * 16;
    os.write(ZEROS, control_end_round - control_end);

    size_t offset = 0;
    for (const auto& m : messages)
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.GetType(i) == typeid(OffsetString))
            {
                auto& ofs = m.Get<OffsetString>(i);
                if (ofs.offset == offset)
                {
                    os.write(ofs.str.c_str(), ofs.str.size() + 1);
                    offset += ofs.str.size() + 1;
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
    foot.msg_descr_size = msg_descr_size;
    foot.count_types = type->item_count;
    foot.field_22 = 0;
    foot.offset_types = msgs_end_round;
    foot.field_28 = field_28;
    foot.offset_msgs = control_end_round;
    foot.field_30 = field_30;
    foot.field_34 = 0;
    foot.field_38 = 0;
    foot.field_3c = 0;
    os.write(reinterpret_cast<char*>(&foot), sizeof(GbnlFooter));
}

namespace
{
struct PrettyType
{
    const char* operator()(uint8_t*)  { return "uint8"; }
    const char* operator()(uint16_t*) { return "uint16"; }
    const char* operator()(uint32_t*) { return "uint32"; }
    const char* operator()(float*)    { return "float"; }
    const char* operator()(Gbnl::OffsetString*) { return "string"; }
};

struct Print
{
    std::ostream& os;
    void operator()(const Gbnl::OffsetString& ofs) { DumpBytes(os, ofs.str); }
    template <typename T> void operator()(T x) { os << x; }
};
}

void Gbnl::Inspect_(std::ostream& os) const
{
    os << "gbnl(" << field_28 << ", " << field_30 << ", types[";

    for (size_t i = 0; i < type->item_count; ++i)
    {
        if (i != 0) os << ", ";
        os << type->Visit<const char*, uint8_t, uint16_t, uint32_t, float, OffsetString>(i, PrettyType{});
    }

    os << "], messages[\n";
    for (const auto& m : messages)
    {
        os << "  (";
        for (size_t i = 0; i < m.GetSize(); ++i)
        {
            if (i != 0) os << ", ";
            m.Visit<void, uint8_t, uint16_t, uint32_t, float, OffsetString>(i, Print{os});
        }
        os << ")\n";
    }
    os << "])";
}

void Gbnl::RecalcSize()
{
    SumSize ss;
    type->ForEach<uint8_t, uint16_t, uint32_t, float, OffsetString>(ss);
    msg_descr_size = ss.len;

    std::map<std::string, size_t> offset_map;
    size_t offset = 0;
    for (auto& m : messages)
    {
        BOOST_ASSERT(m.GetRawType() == type);
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.GetType(i) == typeid(OffsetString))
            {
                auto& os = m.Get<OffsetString>(i);
                auto x = offset_map.emplace(os.str, offset);
                if (x.second) // new item inserted
                    offset += os.str.size() + 1;
                os.offset = x.first->second;
            }
    }
    msgs_size = offset;
}

size_t Gbnl::GetSize() const noexcept
{
    size_t ret = msg_descr_size * messages.size();
    ret = (ret + 15) / 16 * 16 + sizeof(GbnlTypeDescriptor) * type->item_count;
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
    /*
    for (const auto& m : messages)
    {
        os.write(SEP_DASH, sizeof(SEP_DASH));
        os << m.message_id << "\r\n"
           << boost::replace_all_copy(m.text, "\n", "\r\n") << "\r\n";
    }
    os.write(SEP_DASH, sizeof(SEP_DASH));
    os << "EOF";
    */
}

void Gbnl::ReadTxt(std::istream& is)
{
    /*
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
    */
}
