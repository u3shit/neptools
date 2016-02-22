#include "gbnl.hpp"
#include <map>
#include <boost/assert.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/algorithm/string/replace.hpp>

//#define STRTOOL_COMPAT

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

static constexpr size_t Align(size_t val, size_t align)
{
    return (val+align-1) / align * align;
}

static size_t GetSize(uint16_t type)
{
    switch (type)
    {
    case GbnlTypeDescriptor::UINT8: return 1;
    case GbnlTypeDescriptor::UINT16: return 2;
    case GbnlTypeDescriptor::UINT32: return 4;
    case GbnlTypeDescriptor::FLOAT: return 4;
    case GbnlTypeDescriptor::STRING: return 4;
    default: throw std::runtime_error{"GBNL: invalid type"};
    }
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
    {
        auto bytes = ::GetSize(types[i].type);
        calc_offs = ::Align(calc_offs, bytes);
        if (calc_offs != types[i].offset)
            throw std::runtime_error{"GBNL: invalid type offset"};
        calc_offs += bytes;

        switch (types[i].type)
        {
        case GbnlTypeDescriptor::UINT8:
            if (i+1 == foot->count_types || // last item
                types[i+1].offset == ::Align(types[i].offset+1, ::GetSize(types[i+1].type)))
            {
                // single char
                bld.Add<uint8_t>();
            }
            else
            {
                auto len = types[i+1].offset - types[i].offset;
                calc_offs += len-1;
                bld.Add<FixStringTag>(len);
            }
            break;
        case GbnlTypeDescriptor::UINT16:
            bld.Add<uint16_t>();
            break;
        case GbnlTypeDescriptor::UINT32:
            bld.Add<uint32_t>();
            break;
        case GbnlTypeDescriptor::FLOAT:
            bld.Add<float>();
            break;
        case GbnlTypeDescriptor::STRING:
            bld.Add<OffsetString>();
            break;
        default:
            throw std::runtime_error{"GBNL: invalid type"};
        }
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

uint32_t Gbnl::GetId(const Gbnl::Struct& m, size_t i, size_t j, size_t& k) const
{
    size_t this_k;
    if (m.Is<Gbnl::OffsetString>(i))
    {
        if (m.Get<Gbnl::OffsetString>(i).offset == static_cast<size_t>(-1))
            return -1;
        this_k = ++k;
    }
    else if (m.Is<Gbnl::FixStringTag>(i))
        this_k = (flags && field_28 != 1) ? 10000 : 0;
    else
        return -1;

    if (!is_gstl && i == 8 && m.GetSize() == 9 && m.Is<uint32_t>(0)) // todo
        return m.Get<uint32_t>(0);
    else if (is_gstl && m.GetSize() == 3 && m.Is<uint32_t>(1))
    {
#ifdef STRTOOL_COMPAT
        if (i == 0) return -1;
#endif
        return m.Get<uint32_t>(1) + 100000*(i==0);
    }
    else
        return this_k*10000+j;
}

void Gbnl::WriteTxt(std::ostream& os) const
{
    size_t j = 0;
    for (const auto& m : messages)
    {
        size_t k = 0;
        for (size_t i = 0; i < m.GetSize(); ++i)
        {
            auto id = GetId(m, i, j, k);
            if (id != static_cast<uint32_t>(-1))
            {
                std::string str;
                if (m.Is<FixStringTag>(i))
                    str = m.Get<FixStringTag>(i).str;
                else
                    str = m.Get<OffsetString>(i).str;
                boost::replace_all(str, "\n", "\r\n");

#ifdef STRTOOL_COMPAT
                boost::replace_all(str, "#n", "\r\n");
                if (!str.empty())
#endif
                {
                    os.write(SEP_DASH, sizeof(SEP_DASH));
                    os << id << "\r\n" << str << "\r\n";
                }
            }
        }
        ++j;
    }
    os.write(SEP_DASH, sizeof(SEP_DASH));
    os << "EOF";
}

size_t Gbnl::FindDst(uint32_t id, std::vector<Gbnl::Struct>& messages,
                     size_t& index) const
{
    auto size = messages.size();
    for (size_t j = 0; j < size; ++j)
    {
        auto j2 = (index+j) % size;
        auto& m = messages[j2];
        size_t k = 0;
        for (size_t i = 0; i < m.GetSize(); ++i)
        {
            auto tid = GetId(m, i, j2, k);
            if (tid == id)
            {
                index = j2;
                return i;
            }
        }
    }
    return -1;
}

void Gbnl::ReadTxt(std::istream& is)
{
    std::string line, msg;
    size_t last_index = 0, pos = -1;

    while (is.good())
    {
        std::getline(is, line);

        if (line.compare(0, sizeof(SEP_DASH), SEP_DASH, sizeof(SEP_DASH)) == 0)
        {
            if (pos != static_cast<size_t>(-1))
            {
                BOOST_ASSERT(msg.back() == '\n');
                msg.pop_back();
                auto& m = messages[last_index];
                if (m.Is<OffsetString>(pos))
                    m.Get<OffsetString>(pos).str = std::move(msg);
                else
                    strncpy(m.Get<FixStringTag>(pos).str, msg.c_str(),
                            m.GetSize(pos)-1);
                msg.clear();
            }

            if (line.compare(sizeof(SEP_DASH), 3, "EOF") == 0)
            {
                RecalcSize();
                return;
            }
            auto id = std::strtoul(line.data() + sizeof(SEP_DASH), nullptr, 10);
            pos = FindDst(id, messages, last_index);
            if (pos == static_cast<size_t>(-1))
            {
                std::cerr << id << std::endl;
                throw std::runtime_error("GbnlTxt: invalid id in input");
            }
        }
        else
        {
            if (pos == static_cast<size_t>(-1))
                throw std::runtime_error("GbnlTxt: data before separator");
            if (!line.empty() && line.back() == '\r') line.pop_back();
            msg.append(line).append(1, '\n');
        }
    }
    throw std::runtime_error("GbnlTxt: EOF");
}
