#include "gbnl.hpp"
#include <map>
#include <boost/assert.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/algorithm/string/replace.hpp>

bool GbnlFooter::IsValid(size_t chunk_size) const noexcept
{
    if (!(
        field_04 == 1 && field_08 == 16 && field_0c == 4 &&
        descr_offset + msg_descr_size * count_msgs < chunk_size &&
        field_22 == 0 &&
        offset_types + sizeof(GbnlTypeDescriptor) * count_types < chunk_size &&
        offset_msgs < chunk_size &&
        field_34 == 0 && field_38 == 0 && field_3c == 0))
        return false;

    if (memcmp(magic, "GBNL", 4) == 0)
        return descr_offset == 0;
    else if (memcmp(magic, "GSTL", 4) == 0)
        return descr_offset == sizeof(GbnlFooter);
    else
        return false;
}

static void CheckOffs(size_t& val, size_t expect, size_t size, size_t align)
{
    val = (val + align - 1) / align * align;
    if (val != expect)
        throw std::runtime_error{"GBNL: invalid type offset"};
    val += size;
}

Gbnl::Gbnl(const Byte* data, size_t len)
{
    if (len < sizeof(GbnlFooter))
        throw std::runtime_error{"GBNL: section too short"};

    const GbnlFooter* foot;
    if (memcmp(data, "GSTL", 4) == 0)
    {
        foot = reinterpret_cast<const GbnlFooter*>(data);
        is_gstl = true;
    }
    else
    {
        foot = reinterpret_cast<const GbnlFooter*>(
            data + len - sizeof(GbnlFooter));
        is_gstl = false;
    }

    if (!foot->IsValid(len))
        throw std::runtime_error{"GBNL: invalid footer"};
    flags = foot->flags;
    field_28 = foot->field_28;
    field_30 = foot->field_30;

    auto types = reinterpret_cast<const GbnlTypeDescriptor*>(
        data + foot->offset_types);
    msg_descr_size = foot->msg_descr_size;
    size_t calc_offs = 0;

    Struct::TypeBuilder bld;
    for (size_t i = 0; i < foot->count_types; ++i)
        switch (types[i].type)
        {
        case GbnlTypeDescriptor::UINT8:
            if (i+1 == foot->count_types || // last item
                types[i+1].offset == types[i].offset+1)
            {
                // single char
                CheckOffs(calc_offs, types[i].offset, 1, 1);
                bld.Add<uint8_t>();
            }
            else
            {
                auto len = types[i+1].offset - types[i].offset;
                CheckOffs(calc_offs, types[i].offset, len, 1);
                bld.Add<FixStringTag>(len);
            }
            break;
        case GbnlTypeDescriptor::UINT16:
            CheckOffs(calc_offs, types[i].offset, 2, 2);
            bld.Add<uint16_t>();
            break;
        case GbnlTypeDescriptor::UINT32:
            CheckOffs(calc_offs, types[i].offset, 4, 4);
            bld.Add<uint32_t>();
            break;
        case GbnlTypeDescriptor::FLOAT:
            CheckOffs(calc_offs, types[i].offset, 4, 4);
            bld.Add<float>();
            break;
        case GbnlTypeDescriptor::STRING:
            CheckOffs(calc_offs, types[i].offset, 4, 4);
            bld.Add<OffsetString>();
            break;
        default:
            throw std::runtime_error{"GBNL: invalid type"};
        }
    if (((calc_offs + 3) & ~3) != msg_descr_size)
        throw std::runtime_error{"GBNL: type array incomplete/bad padding"};

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
            switch (type->items[i].idx)
            {
            case Struct::GetIndexFromType<uint8_t>():
                m.Get<uint8_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint8_t*>(ptr);
                break;
            case Struct::GetIndexFromType<uint16_t>():
                m.Get<uint16_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint16_t*>(ptr);
                break;
            case Struct::GetIndexFromType<uint32_t>():
                m.Get<uint32_t>(i) = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                break;
            case Struct::GetIndexFromType<float>():
            {
                uint32_t x = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                m.Get<float>(i) = *reinterpret_cast<float*>(&x);
                break;
            }
            case Struct::GetIndexFromType<OffsetString>():
            {
                uint32_t offs = *reinterpret_cast<
                    const boost::endian::little_uint32_t*>(ptr);
                if (offs == 0xffffffff)
                    m.Get<OffsetString>(i).offset = -1;
                else
                {
                    if (offs > len - foot->offset_msgs)
                        throw std::runtime_error{"GBNL: string offset too big"};
                    auto str = reinterpret_cast<const char*>(
                        data + foot->offset_msgs + offs);

                    m.Get<OffsetString>(i) = {str, 0};
                }
                break;
            }
            case Struct::GetIndexFromType<FixStringTag>():
            {
                auto len = type->items[i].size;
                auto dst = m.Get<FixStringTag>(i).str;
                strncpy(dst, reinterpret_cast<const char*>(ptr), len-1);
                dst[len-1] = '\0';
                break;
            }
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
    WriteDescr(char* ptr) : ptr{ptr} {}
    char* ptr;
    size_t offs = 0;
    void operator()(uint8_t x, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint8_t*>(ptr+offs) = x;
        offs += 1;
    }
    void operator()(uint16_t x, size_t)
    {
        offs = (offs+1) & ~1;
        *reinterpret_cast<boost::endian::little_uint16_t*>(ptr+offs) = x;
        offs += 2;
    }
    void operator()(uint32_t x, size_t)
    {
        offs = (offs+3) & ~3;
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr+offs) = x;
        offs += 4;
    }
    void operator()(float y, size_t)
    {
        offs = (offs+3) & ~3;
        uint32_t x = *reinterpret_cast<uint32_t*>(&y);
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr+offs) = x;
        offs += 4;
    }
    void operator()(const Gbnl::OffsetString& os, size_t)
    {
        offs = (offs+3) & ~3;
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr+offs) = os.offset;
        offs += 4;
    }
    void operator()(const Gbnl::FixStringTag& fs, size_t len)
    {
        memset(ptr+offs, 0, len);
        strncpy(ptr+offs, fs.str, len-1);
        offs += len;
    }
};
}

static char ZEROS[16];
void Gbnl::Dump_(std::ostream& os) const
{
    if (is_gstl) DumpHeader(os);

    char* msgd = static_cast<char*>(alloca(msg_descr_size));
    memset(msgd, 0, msg_descr_size);

    for (const auto& m : messages)
    {
        m.ForEach(WriteDescr{msgd});
        os.write(msgd, msg_descr_size);
    }

    auto msgs_end = msg_descr_size * messages.size();
    auto msgs_end_round = Align(msgs_end);
    os.write(ZEROS, msgs_end_round - msgs_end);

    GbnlTypeDescriptor ctrl;
    uint16_t offs = 0;
    for (size_t i = 0; i < type->item_count; ++i)
    {
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::UINT8;
            offs += 1;
            break;
        case Struct::GetIndexFromType<uint16_t>():
            offs = (offs+1) & ~1;
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::UINT16;
            offs += 2;
            break;
        case Struct::GetIndexFromType<uint32_t>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::UINT32;
            offs += 4;
            break;
        case Struct::GetIndexFromType<float>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::FLOAT;
            offs += 4;
            break;
        case Struct::GetIndexFromType<OffsetString>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::STRING;
            offs += 4;
            break;
        case Struct::GetIndexFromType<FixStringTag>():
            ctrl.offset = offs;
            ctrl.type = GbnlTypeDescriptor::UINT8;
            offs += type->items[i].size;
            break;
        };
        os.write(reinterpret_cast<char*>(&ctrl), sizeof(GbnlTypeDescriptor));
    }
    auto control_end = msgs_end_round + sizeof(GbnlTypeDescriptor) * type->item_count;
    auto control_end_round = Align(control_end);
    os.write(ZEROS, control_end_round - control_end);

    size_t offset = 0;
    for (const auto& m : messages)
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<OffsetString>(i))
            {
                auto& ofs = m.Get<OffsetString>(i);
                if (ofs.offset == offset)
                {
                    os.write(ofs.str.c_str(), ofs.str.size() + 1);
                    offset += ofs.str.size() + 1;
                }
            }

    BOOST_ASSERT(offset == msgs_size);
    auto offset_round = Align(offset);
    os.write(ZEROS, offset_round - offset);

    BOOST_ASSERT(msgs_end_round == Align(msg_descr_size * messages.size()));
    BOOST_ASSERT(control_end_round == Align(msgs_end_round +
        sizeof(GbnlTypeDescriptor) * type->item_count));
    if (!is_gstl) DumpHeader(os);
}

void Gbnl::DumpHeader(std::ostream& os) const
{
    GbnlFooter foot;
    memcpy(foot.magic, is_gstl ? "GSTL" : "GBNL", 4);
    foot.field_04 = 1;
    foot.field_08 = 16;
    foot.field_0c = 4;
    foot.flags = flags;
    auto offset = is_gstl ? sizeof(GbnlFooter) : 0;
    foot.descr_offset = offset;
    foot.count_msgs = messages.size();
    foot.msg_descr_size = msg_descr_size;
    foot.count_types = type->item_count;
    foot.field_22 = 0;
    auto msgs_end_round = Align(offset + msg_descr_size * messages.size());
    foot.offset_types = msgs_end_round;;
    foot.field_28 = field_28;
    auto control_end_round = Align(msgs_end_round +
        sizeof(GbnlTypeDescriptor) * type->item_count);
    foot.offset_msgs = msgs_size ? control_end_round : 0;
    foot.field_30 = field_30;
    foot.field_34 = 0;
    foot.field_38 = 0;
    foot.field_3c = 0;
    os.write(reinterpret_cast<char*>(&foot), sizeof(GbnlFooter));
}

namespace
{
struct Print
{
    std::ostream& os;
    void operator()(const Gbnl::OffsetString& ofs, size_t)
    {
        if (ofs.offset == static_cast<size_t>(-1))
            os << "null";
        else
            DumpBytes(os, ofs.str);
    }
    void operator()(const Gbnl::FixStringTag& fs, size_t) { DumpBytes(os, fs.str); }
    void operator()(uint8_t x, size_t) { os << static_cast<unsigned>(x); }
    template <typename T> void operator()(T x, size_t) { os << x; }
};
}

void Gbnl::Inspect_(std::ostream& os) const
{
    os << (is_gstl ? "gstl(" : "gbnl(") << flags << ", " << field_28 << ", "
       << field_30 << ", types[";

    for (size_t i = 0; i < type->item_count; ++i)
    {
        if (i != 0) os << ", ";
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():      os << "uint8";  break;
        case Struct::GetIndexFromType<uint16_t>():     os << "uint16"; break;
        case Struct::GetIndexFromType<uint32_t>():     os << "uint32"; break;
        case Struct::GetIndexFromType<float>():        os << "float";  break;
        case Struct::GetIndexFromType<OffsetString>(): os << "string"; break;
        case Struct::GetIndexFromType<FixStringTag>():
            os << "fix_string(" << type->items[i].size << ")";
            break;
        }
    }

    os << "], messages[\n";
    for (const auto& m : messages)
    {
        os << "  (";
        for (size_t i = 0; i < m.GetSize(); ++i)
        {
            if (i != 0) os << ", ";
            m.Visit<void>(i, Print{os});
        }
        os << ")\n";
    }
    os << "])";
}

void Gbnl::RecalcSize()
{
    size_t len = 0;
    for (size_t i = 0; i < type->item_count; ++i)
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():      len += 1; break;
        case Struct::GetIndexFromType<uint16_t>():     len = (len+2+1) & ~1; break;
        case Struct::GetIndexFromType<uint32_t>():     len = (len+4+3) & ~3; break;
        case Struct::GetIndexFromType<float>():        len = (len+4+3) & ~3; break;
        case Struct::GetIndexFromType<OffsetString>(): len = (len+4+3) & ~3; break;
        case Struct::GetIndexFromType<FixStringTag>(): len += type->items[i].size; break;
        }
    msg_descr_size = (len+3) & ~3;

    std::map<std::string, size_t> offset_map;
    size_t offset = 0;
    for (auto& m : messages)
    {
        BOOST_ASSERT(m.GetRawType() == type);
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<OffsetString>(i))
            {
                auto& os = m.Get<OffsetString>(i);
                if (os.offset == static_cast<size_t>(-1)) continue;
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
    ret = Align(ret) + sizeof(GbnlTypeDescriptor) * type->item_count;
    ret = Align(ret) + msgs_size;
    ret = Align(ret) + sizeof(GbnlFooter);
    return ret;
}

size_t Gbnl::Align(size_t x) const noexcept
{
    return is_gstl ? x : ((x+15) & ~15);
}

const char SEP_DASH[] = {
#define REP_MACRO(x,y,z) char(0x81), char(0x5c),
    BOOST_PP_REPEAT(40, REP_MACRO, )
    ' '
};

static uint32_t GetId(const Gbnl::Struct& m, size_t i, size_t j, size_t k)
{
    if (i == 8 && m.GetSize() == 9 && m.Is<uint32_t>(0)) // todo
        return m.Get<uint32_t>(0);
    else
        return k*10000+j;
}

void Gbnl::WriteTxt(std::ostream& os) const
{
    size_t j = 0;
    for (const auto& m : messages)
    {
        size_t k = 0;
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<OffsetString>(i))
            {
                os.write(SEP_DASH, sizeof(SEP_DASH));
                os << GetId(m, i, j, ++k) << "\r\n"
                   << boost::replace_all_copy(m.Get<OffsetString>(i).str, "\n", "\r\n")
                   << "\r\n";
            }
        ++j;
    }
    os.write(SEP_DASH, sizeof(SEP_DASH));
    os << "EOF";
}

static Gbnl::OffsetString* FindDst(
    uint32_t id, std::vector<Gbnl::Struct>& messages, size_t& index)
{
    auto size = messages.size();
    for (size_t j = 0; j < size; ++j)
    {
        auto j2 = (index+j) % size;
        auto& m = messages[j2];
        size_t k = 0;
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<Gbnl::OffsetString>(i) && GetId(m, i, j2, ++k) == id)
            {
                index = j2;
                return &m.Get<Gbnl::OffsetString>(i);
            }
    }
    return nullptr;
}

void Gbnl::ReadTxt(std::istream& is)
{
    OffsetString* dst = nullptr;
    std::string line, msg;
    size_t last_index = 0;

    while (is.good())
    {
        std::getline(is, line);

        if (line.compare(0, sizeof(SEP_DASH), SEP_DASH, sizeof(SEP_DASH)) == 0)
        {
            if (dst)
            {
                BOOST_ASSERT(msg.back() == '\n');
                msg.pop_back();
                dst->str = std::move(msg);
                msg.clear();
            }

            if (line.compare(sizeof(SEP_DASH), 3, "EOF") == 0)
            {
                RecalcSize();
                return;
            }
            auto id = std::strtoul(line.data() + sizeof(SEP_DASH), nullptr, 10);
            dst = FindDst(id, messages, last_index);
            if (!dst)
                throw std::runtime_error("GbnlTxt: invalid id in input");
        }
        else
        {
            if (!dst)
                throw std::runtime_error("GbnlTxt: data before separator");
            if (!line.empty() && line.back() == '\r') line.pop_back();
            msg.append(line).append(1, '\n');
        }
    }
    throw std::runtime_error("GbnlTxt: EOF");
}
