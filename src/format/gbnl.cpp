#include "gbnl.hpp"
#include "../except.hpp"
#include <map>
#include <boost/preprocessor/repetition/repeat.hpp>

#include <boost/algorithm/string/replace.hpp>

//#define STRTOOL_COMPAT

namespace Neptools
{

void Gbnl::Header::Validate(size_t chunk_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Gbnl::Header", x)
    VALIDATE(field_04 == 1 && field_08 == 16 && field_0c == 4);
    VALIDATE(descr_offset + msg_descr_size * count_msgs < chunk_size);
    VALIDATE(field_22 == 0);
    VALIDATE(offset_types + sizeof(TypeDescriptor) * count_types < chunk_size);
    VALIDATE(offset_msgs < chunk_size);
    VALIDATE(field_34 == 0 && field_38 == 0 && field_3c == 0);

    if (memcmp(magic, "GBNL", 4) == 0)
        VALIDATE(descr_offset == 0);
    else if (memcmp(magic, "GSTL", 4) == 0)
        VALIDATE(descr_offset == sizeof(Header));
    else
        VALIDATE(!"Invalid magic");
#undef VALIDATE
}

static constexpr size_t Align(size_t val, size_t align)
{
    return (val+align-1) / align * align;
}

static size_t GetSize(uint16_t type)
{
    switch (type)
    {
    case Gbnl::TypeDescriptor::UINT8: return 1;
    case Gbnl::TypeDescriptor::UINT16: return 2;
    case Gbnl::TypeDescriptor::UINT32: return 4;
    case Gbnl::TypeDescriptor::FLOAT: return 4;
    case Gbnl::TypeDescriptor::STRING: return 4;
    default: NEPTOOLS_THROW(DecodeError{"Gbnl: invalid type"});
    }
}

Gbnl::Gbnl(Source src)
{
    AddInfo(&Gbnl::Parse_, ADD_SOURCE(src), this, src);
}

void Gbnl::Parse_(Source& src)
{
#define VALIDATE(msg, x) NEPTOOLS_VALIDATE_FIELD("Gbnl" msg, x)

    src.CheckSize(sizeof(Header));
    auto foot = src.Pread<Header>(0);
    if (memcmp(foot.magic, "GSTL", 4) == 0)
    {
        is_gstl = true;
    }
    else
    {
        src.Pread(src.GetSize() - sizeof(Header), foot);
        is_gstl = false;
    }

    foot.Validate(src.GetSize());
    flags = foot.flags;
    field_28 = foot.field_28;
    field_30 = foot.field_30;

    src.Seek(foot.offset_types);
    msg_descr_size = foot.msg_descr_size;
    size_t calc_offs = 0;

    std::vector<uint16_t> offsets;
    offsets.reserve(foot.count_types);
    Struct::TypeBuilder bld;
    for (size_t i = 0; i < foot.count_types; ++i)
    {
        auto type = src.Read<TypeDescriptor>();

        auto bytes = ::Neptools::GetSize(type.type);
        calc_offs = ::Neptools::Align(calc_offs, bytes);
        VALIDATE("", calc_offs == type.offset);
        calc_offs += bytes;
        offsets.push_back(type.offset);

        switch (type.type)
        {
        case TypeDescriptor::UINT8:
            if (i+1 != foot.count_types)
            {
                auto t2 = src.Pread<TypeDescriptor>(src.Tell());
                // if it longer than 1, make it a string
                if (t2.offset != ::Neptools::Align(
                        type.offset+1, ::Neptools::GetSize(t2.type)))
                {
                    auto len = t2.offset - type.offset;
                    calc_offs += len-1;
                    bld.Add<FixStringTag>(len);
                    break;
                }
            }

            // single char
            bld.Add<uint8_t>();
            break;
        case TypeDescriptor::UINT16:
            bld.Add<uint16_t>();
            break;
        case TypeDescriptor::UINT32:
            bld.Add<uint32_t>();
            break;
        case TypeDescriptor::FLOAT:
            bld.Add<float>();
            break;
        case TypeDescriptor::STRING:
            bld.Add<OffsetString>();
            break;
        default:
            NEPTOOLS_THROW(DecodeError{"GBNL: invalid type"});
        }
    }
    VALIDATE(" type array incomplete/bad padding",
             ((calc_offs + 3) & ~3) == msg_descr_size);

    type = bld.Build();

    auto msgs = foot.descr_offset;
    messages.reserve(foot.count_msgs);
    for (size_t i = 0; i < foot.count_msgs; ++i)
    {
        messages.emplace_back(type);
        auto& m = messages.back();
        for (size_t i = 0; i < foot.count_types; ++i)
        {
            src.Seek(msgs + offsets[i]);
            switch (type->items[i].idx)
            {
            case Struct::GetIndexFromType<uint8_t>():
                m.Get<uint8_t>(i) = src.GetLittleUint8();
                break;
            case Struct::GetIndexFromType<uint16_t>():
                m.Get<uint16_t>(i) = src.GetLittleUint16();
                break;
            case Struct::GetIndexFromType<uint32_t>():
                m.Get<uint32_t>(i) = src.GetLittleUint32();
                break;
            case Struct::GetIndexFromType<float>():
            {
                union { float f; uint32_t i; } x;
                x.i = src.GetLittleUint32();
                m.Get<float>(i) = x.f;
                break;
            }
            case Struct::GetIndexFromType<OffsetString>():
            {
                uint32_t offs = src.GetLittleUint32();
                if (offs == 0xffffffff)
                    m.Get<OffsetString>(i).offset = -1;
                else
                {
                    VALIDATE("", offs < src.GetSize() - foot.offset_msgs);
                    auto str = foot.offset_msgs + offs;

                    m.Get<OffsetString>(i) = {src.GetCstring(str), 0};
                }
                break;
            }
            case Struct::GetIndexFromType<FixStringTag>():
            {
                auto len = type->items[i].size;
                auto dst = m.Get<FixStringTag>(i).str;
                src.Read(dst, len);
                break;
            }
            }
        }

        msgs += msg_descr_size;
    }
    RecalcSize();

    VALIDATE(" invalid size after repack", msg_descr_size == foot.msg_descr_size);
    VALIDATE(" invalid size after repack", GetSize() == src.GetSize());
#undef VALIDATE
}

namespace
{
struct WriteDescr
{
    WriteDescr(Byte* ptr) : ptr{ptr} {}
    Byte* ptr;
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
        strncpy(reinterpret_cast<char*>(ptr+offs), fs.str, len-1);
        offs += len;
    }
};
}

void Gbnl::Dump_(Sink& sink) const
{
    if (is_gstl) DumpHeader(sink);

    Byte msgd[msg_descr_size];
    memset(msgd, 0, msg_descr_size);

    for (const auto& m : messages)
    {
        m.ForEach(WriteDescr{msgd});
        sink.Write(msgd, msg_descr_size);
    }

    auto msgs_end = msg_descr_size * messages.size();
    auto msgs_end_round = Align(msgs_end);
    sink.Pad(msgs_end_round - msgs_end);

    TypeDescriptor ctrl;
    uint16_t offs = 0;
    for (size_t i = 0; i < type->item_count; ++i)
    {
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::UINT8;
            offs += 1;
            break;
        case Struct::GetIndexFromType<uint16_t>():
            offs = (offs+1) & ~1;
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::UINT16;
            offs += 2;
            break;
        case Struct::GetIndexFromType<uint32_t>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::UINT32;
            offs += 4;
            break;
        case Struct::GetIndexFromType<float>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::FLOAT;
            offs += 4;
            break;
        case Struct::GetIndexFromType<OffsetString>():
            offs = (offs+3) & ~3;
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::STRING;
            offs += 4;
            break;
        case Struct::GetIndexFromType<FixStringTag>():
            ctrl.offset = offs;
            ctrl.type = TypeDescriptor::UINT8;
            offs += type->items[i].size;
            break;
        };
        sink.Write(ctrl);
    }
    auto control_end = msgs_end_round + sizeof(TypeDescriptor) * type->item_count;
    auto control_end_round = Align(control_end);
    sink.Pad(control_end_round - control_end);

    size_t offset = 0;
    for (const auto& m : messages)
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<OffsetString>(i))
            {
                auto& ofs = m.Get<OffsetString>(i);
                if (ofs.offset == offset)
                {
                    sink.WriteCString(ofs.str);
                    offset += ofs.str.size() + 1;
                }
            }

    NEPTOOLS_ASSERT(offset == msgs_size);
    auto offset_round = Align(offset);
    sink.Pad(offset_round - offset);

    // sanity checks
    NEPTOOLS_ASSERT(msgs_end_round == Align(msg_descr_size * messages.size()));
    NEPTOOLS_ASSERT(control_end_round == Align(msgs_end_round +
        sizeof(TypeDescriptor) * type->item_count));
    if (!is_gstl) DumpHeader(sink);
}

void Gbnl::DumpHeader(Sink& sink) const
{
    Header head;
    memcpy(head.magic, is_gstl ? "GSTL" : "GBNL", 4);
    head.field_04 = 1;
    head.field_08 = 16;
    head.field_0c = 4;
    head.flags = flags;
    auto offset = is_gstl ? sizeof(Header) : 0;
    head.descr_offset = offset;
    head.count_msgs = messages.size();
    head.msg_descr_size = msg_descr_size;
    head.count_types = type->item_count;
    head.field_22 = 0;
    auto msgs_end_round = Align(offset + msg_descr_size * messages.size());
    head.offset_types = msgs_end_round;;
    head.field_28 = field_28;
    auto control_end_round = Align(msgs_end_round +
        sizeof(TypeDescriptor) * type->item_count);
    head.offset_msgs = msgs_size ? control_end_round : 0;
    head.field_30 = field_30;
    head.field_34 = 0;
    head.field_38 = 0;
    head.field_3c = 0;
    sink.Write(head);
}

namespace
{
struct Print
{
    std::ostream& os;
    void operator()(const Gbnl::OffsetString& ofs, size_t)
    {
        if (ofs.offset == static_cast<uint32_t>(-1))
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
        NEPTOOLS_ASSERT(m.GetRawType() == type);
        for (size_t i = 0; i < m.GetSize(); ++i)
            if (m.Is<OffsetString>(i))
            {
                auto& os = m.Get<OffsetString>(i);
                if (os.offset == static_cast<uint32_t>(-1)) continue;
                auto x = offset_map.emplace(os.str, offset);
                if (x.second) // new item inserted
                    offset += os.str.size() + 1;
                os.offset = x.first->second;
            }
    }
    msgs_size = offset;
}

FilePosition Gbnl::GetSize() const noexcept
{
    FilePosition ret = msg_descr_size * messages.size();
    ret = Align(ret) + sizeof(TypeDescriptor) * type->item_count;
    ret = Align(ret) + msgs_size;
    ret = Align(ret) + sizeof(Header);
    return ret;
}

FilePosition Gbnl::Align(FilePosition x) const noexcept
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
        if (m.Get<Gbnl::OffsetString>(i).offset == static_cast<uint32_t>(-1))
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
                NEPTOOLS_ASSERT(msg.back() == '\n');
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
            uint32_t id = std::strtoul(line.data() + sizeof(SEP_DASH), nullptr, 10);
            pos = FindDst(id, messages, last_index);
            if (pos == static_cast<size_t>(-1))
            {
                NEPTOOLS_THROW(DecodeError{"GbnlTxt: invalid id in input"} <<
                      FailedId{id});
            }
        }
        else
        {
            if (pos == static_cast<size_t>(-1))
                NEPTOOLS_THROW(DecodeError{"GbnlTxt: data before separator"});
            if (!line.empty() && line.back() == '\r') line.pop_back();
            msg.append(line).append(1, '\n');
        }
    }
    NEPTOOLS_THROW(DecodeError{"GbnlTxt: EOF"});
}

}
