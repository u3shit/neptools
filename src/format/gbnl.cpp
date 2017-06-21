#include "gbnl.hpp"
#include "../dynamic_struct.lua.hpp"
#include "../except.hpp"
#include "../sink.hpp"

#include <map>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

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

static size_t GetSize(uint16_t type)
{
    switch (type)
    {
    case Gbnl::TypeDescriptor::UINT8: return 1;
    case Gbnl::TypeDescriptor::UINT16: return 2;
    case Gbnl::TypeDescriptor::UINT32: return 4;
    case Gbnl::TypeDescriptor::UINT64: return 8;
    case Gbnl::TypeDescriptor::FLOAT: return 4;
    case Gbnl::TypeDescriptor::STRING: return 4;
    }
    NEPTOOLS_THROW(DecodeError{"Gbnl: invalid type"});
}

Gbnl::Gbnl(Source src)
{
    AddInfo(&Gbnl::Parse_, ADD_SOURCE(src), this, src);
}

void Gbnl::Parse_(Source& src)
{
#define VALIDATE(msg, x) NEPTOOLS_VALIDATE_FIELD("Gbnl" msg, x)

    src.CheckSize(sizeof(Header));
    auto foot = src.PreadGen<Header>(0);
    if (memcmp(foot.magic, "GSTL", 4) == 0)
    {
        is_gstl = true;
    }
    else
    {
        src.PreadGen(src.GetSize() - sizeof(Header), foot);
        is_gstl = false;
    }

    foot.Validate(src.GetSize());
    flags = foot.flags;
    field_28 = foot.field_28;
    field_30 = foot.field_30;

    src.Seek(foot.offset_types);
    msg_descr_size = foot.msg_descr_size;
    size_t calc_offs = 0;

    //std::vector<uint16_t> offsets;
    //offsets.reserve(foot.count_types);
    Struct::TypeBuilder bld;
    bool uint8_in_progress = false;
    for (size_t i = 0; i < foot.count_types; ++i)
    {
        auto type = src.ReadGen<TypeDescriptor>();
        VALIDATE("unordered types", calc_offs <= type.offset);

        Pad(type.offset - calc_offs, bld, uint8_in_progress);
        calc_offs = type.offset + ::Neptools::GetSize(type.type);

        switch (type.type)
        {
        case TypeDescriptor::UINT8:
            NEPTOOLS_ASSERT(!uint8_in_progress);
            uint8_in_progress = true;
            break;
        case TypeDescriptor::UINT16:
            bld.Add<uint16_t>();
            break;
        case TypeDescriptor::UINT32:
            bld.Add<uint32_t>();
            break;
        case TypeDescriptor::UINT64:
            bld.Add<uint64_t>();
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
    Pad(msg_descr_size - calc_offs, bld, uint8_in_progress);

    type = bld.Build();

    auto msgs = foot.descr_offset;
    messages.reserve(foot.count_msgs);
    for (size_t i = 0; i < foot.count_msgs; ++i)
    {
        messages.emplace_back(Struct::New(type));
        auto& m = messages.back();
        src.Seek(msgs);
        for (size_t i = 0; i < type->item_count; ++i)
        {
            switch (type->items[i].idx)
            {
            case Struct::GetIndexFromType<uint8_t>():
                m->Get<uint8_t>(i) = src.ReadLittleUint8();
                break;
            case Struct::GetIndexFromType<uint16_t>():
                m->Get<uint16_t>(i) = src.ReadLittleUint16();
                break;
            case Struct::GetIndexFromType<uint32_t>():
                m->Get<uint32_t>(i) = src.ReadLittleUint32();
                break;
            case Struct::GetIndexFromType<uint64_t>():
                m->Get<uint64_t>(i) = src.ReadLittleUint64();
                break;
            case Struct::GetIndexFromType<float>():
            {
                union { float f; uint32_t i; } x;
                x.i = src.ReadLittleUint32();
                m->Get<float>(i) = x.f;
                break;
            }
            case Struct::GetIndexFromType<OffsetString>():
            {
                uint32_t offs = src.ReadLittleUint32();
                if (offs == 0xffffffff)
                    m->Get<OffsetString>(i).offset = -1;
                else
                {
                    VALIDATE("", offs < src.GetSize() - foot.offset_msgs);
                    auto str = foot.offset_msgs + offs;

                    m->Get<OffsetString>(i) = {src.PreadCString(str), 0};
                }
                break;
            }
            case Struct::GetIndexFromType<FixStringTag>():
                src.Read(m->Get<FixStringTag>(i).str, type->items[i].size);
                break;
            case Struct::GetIndexFromType<PaddingTag>():
                src.Read(m->Get<PaddingTag>(i).pad, type->items[i].size);
                break;
            }
        }

        msgs += msg_descr_size;
    }
    RecalcSize();

    VALIDATE(" invalid size after repack", msg_descr_size == foot.msg_descr_size);
    VALIDATE(" invalid size after repack", GetSize() == src.GetSize());
#undef VALIDATE
}

void Gbnl::Pad(uint16_t diff, Struct::TypeBuilder& bld, bool& uint8_in_progress)
{
    if (uint8_in_progress)
    {
        uint8_in_progress = false;
        if (diff <= 3) // probably padding
            bld.Add<uint8_t>();
        else
        {
            bld.Add<FixStringTag>(diff+1);
            return;
        }
    }

    if (diff) bld.Add<PaddingTag>(diff);
}

namespace
{
struct WriteDescr
{
    WriteDescr(Byte* ptr) : ptr{ptr} {}
    Byte* ptr;
    // todo: template
    void operator()(uint8_t x, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint8_t*>(ptr) = x;
        ptr += 1;
    }
    void operator()(uint16_t x, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint16_t*>(ptr) = x;
        ptr += 2;
    }
    void operator()(uint32_t x, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = x;
        ptr += 4;
    }
    void operator()(uint64_t x, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint64_t*>(ptr) = x;
        ptr += 8;
    }
    void operator()(float y, size_t)
    {
        NEPTOOLS_STATIC_ASSERT(sizeof(float) == sizeof(uint32_t));
        union { float f; uint32_t i; } x;
        x.f = y;
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = x.i;
        ptr += 4;
    }
    void operator()(const Gbnl::OffsetString& os, size_t)
    {
        *reinterpret_cast<boost::endian::little_uint32_t*>(ptr) = os.offset;
        ptr += 4;
    }
    void operator()(const Gbnl::FixStringTag& fs, size_t len)
    {
        memset(ptr, 0, len);
        strncpy(reinterpret_cast<char*>(ptr), fs.str, len-1);
        ptr += len;
    }
    void operator()(const Gbnl::PaddingTag& pd, size_t len)
    {
        memcpy(ptr, pd.pad, len);
        ptr += len;
    }
};
}

void Gbnl::Dump_(Sink& sink) const
{
    if (is_gstl) DumpHeader(sink);

    // RB2-3 scripts: 36
    // VII scrips: 392
    // gbin/gstrs are usually smaller than VII scripts
    // RB3's stdungeon.gbin: 7576, stsqdungeon.gbin: 1588 though
    //std::cerr << msg_descr_size << std::endl;
    boost::container::small_vector<Byte, 392> msgd;
    msgd.resize(msg_descr_size);

    for (const auto& m : messages)
    {
        m->ForEach(WriteDescr{msgd.data()});
        sink.Write({msgd.data(), msg_descr_size});
    }

    auto msgs_end = msg_descr_size * messages.size();
    auto msgs_end_round = Align(msgs_end);
    sink.Pad(msgs_end_round - msgs_end);

    TypeDescriptor ctrl;
    uint16_t offs = 0;
    for (size_t i = 0; i < type->item_count; ++i)
    {
        ctrl.offset = offs;
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():
            ctrl.type = TypeDescriptor::UINT8;
            offs += 1;
            break;
        case Struct::GetIndexFromType<uint16_t>():
            ctrl.type = TypeDescriptor::UINT16;
            offs += 2;
            break;
        case Struct::GetIndexFromType<uint32_t>():
            ctrl.type = TypeDescriptor::UINT32;
            offs += 4;
            break;
        case Struct::GetIndexFromType<uint64_t>():
            ctrl.type = TypeDescriptor::UINT64;
            offs += 8;
            break;
        case Struct::GetIndexFromType<float>():
            ctrl.type = TypeDescriptor::FLOAT;
            offs += 4;
            break;
        case Struct::GetIndexFromType<OffsetString>():
            ctrl.type = TypeDescriptor::STRING;
            offs += 4;
            break;
        case Struct::GetIndexFromType<FixStringTag>():
            ctrl.type = TypeDescriptor::UINT8;
            offs += type->items[i].size;
            break;
        case Struct::GetIndexFromType<PaddingTag>():
            offs += type->items[i].size;
            goto skip;
        }
        sink.WriteGen(ctrl);
    skip: ;
    }
    auto control_end = msgs_end_round + sizeof(TypeDescriptor) * real_item_count;
    auto control_end_round = Align(control_end);
    sink.Pad(control_end_round - control_end);

    size_t offset = 0;
    for (const auto& m : messages)
        for (size_t i = 0; i < m->GetSize(); ++i)
            if (m->Is<OffsetString>(i))
            {
                auto& ofs = m->Get<OffsetString>(i);
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
        sizeof(TypeDescriptor) * real_item_count));
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
    head.count_types = real_item_count;
    head.field_22 = 0;
    auto msgs_end_round = Align(offset + msg_descr_size * messages.size());
    head.offset_types = msgs_end_round;;
    head.field_28 = field_28;
    auto control_end_round = Align(msgs_end_round +
        sizeof(TypeDescriptor) * real_item_count);
    head.offset_msgs = msgs_size ? control_end_round : 0;
    head.field_30 = field_30;
    head.field_34 = 0;
    head.field_38 = 0;
    head.field_3c = 0;
    sink.WriteGen(head);
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
    void operator()(const Gbnl::FixStringTag& fs, size_t)
    { DumpBytes(os, fs.str); }
    void operator()(const Gbnl::PaddingTag& pd, size_t size)
    { DumpBytes(os, {pd.pad, size}); }
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
        case Struct::GetIndexFromType<uint64_t>():     os << "uint64"; break;
        case Struct::GetIndexFromType<float>():        os << "float";  break;
        case Struct::GetIndexFromType<OffsetString>(): os << "string"; break;
        case Struct::GetIndexFromType<FixStringTag>():
            os << "fix_string(" << type->items[i].size << ")";
            break;
        case Struct::GetIndexFromType<PaddingTag>():
            os << "pad(" << type->items[i].size << ")";
            break;
        }
    }

    os << "], messages[\n";
    for (const auto& m : messages)
    {
        os << "  (";
        for (size_t i = 0; i < m->GetSize(); ++i)
        {
            if (i != 0) os << ", ";
            m->Visit<void>(i, Print{os});
        }
        os << ")\n";
    }
    os << "])";
}

void Gbnl::RecalcSize()
{
    size_t len = 0, count = 0;
    for (size_t i = 0; i < type->item_count; ++i)
        switch (type->items[i].idx)
        {
        case Struct::GetIndexFromType<uint8_t>():      len += 1; ++count; break;
        case Struct::GetIndexFromType<uint16_t>():     len += 2; ++count; break;
        case Struct::GetIndexFromType<uint32_t>():     len += 4; ++count; break;
        case Struct::GetIndexFromType<uint64_t>():     len += 8; ++count; break;
        case Struct::GetIndexFromType<float>():        len += 4; ++count; break;
        case Struct::GetIndexFromType<OffsetString>(): len += 4; ++count; break;
        case Struct::GetIndexFromType<FixStringTag>():
            len += type->items[i].size; ++count; break;
        case Struct::GetIndexFromType<PaddingTag>():
            len += type->items[i].size; break;
        }
    msg_descr_size = len;
    real_item_count = count;

    std::map<std::string, size_t> offset_map;
    size_t offset = 0;
    for (auto& m : messages)
    {
        NEPTOOLS_ASSERT(m->GetType() == type);
        for (size_t i = 0; i < m->GetSize(); ++i)
            if (m->Is<OffsetString>(i))
            {
                auto& os = m->Get<OffsetString>(i);
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
    ret = Align(ret) + sizeof(TypeDescriptor) * real_item_count;
    ret = Align(ret) + msgs_size;
    ret = Align(ret) + sizeof(Header);
    return ret;
}

FilePosition Gbnl::Align(FilePosition x) const noexcept
{
    return is_gstl ? x : ((x+15) & ~15);
}

static const char SEP_DASH_DATA[] = {
#define REP_MACRO(x,y,z) char(0x81), char(0x5c),
    BOOST_PP_REPEAT(40, REP_MACRO, )
#undef REP_MACRO
    ' '
};
static const StringView SEP_DASH{SEP_DASH_DATA, sizeof(SEP_DASH_DATA)};

static const char SEP_DASH_UTF8_DATA[] = {
#define REP_MACRO(x,y,z) char(0xe2), char(0x80), char(0x95),
    BOOST_PP_REPEAT(40, REP_MACRO, )
#undef REP_MACRO
    ' '
};
static const StringView SEP_DASH_UTF8{
    SEP_DASH_UTF8_DATA, sizeof(SEP_DASH_UTF8_DATA)};


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

    // hack
    if (!is_gstl && m.Is<uint32_t>(0) && (
            (i == 8 && m.GetSize() == 9) || // rebirths
            (i == 105 && m.GetSize() == 107))) // vii
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

void Gbnl::WriteTxt_(std::ostream& os) const
{
    auto sep = field_30 == 8 ? SEP_DASH_UTF8 : SEP_DASH;
    size_t j = 0;
    for (const auto& m : messages)
    {
        size_t k = 0;
        for (size_t i = 0; i < m->GetSize(); ++i)
        {
            auto id = GetId(*m, i, j, k);
            if (id != static_cast<uint32_t>(-1))
            {
                std::string str;
                if (m->Is<FixStringTag>(i))
                    str = m->Get<FixStringTag>(i).str;
                else
                    str = m->Get<OffsetString>(i).str;
                boost::replace_all(str, "\n", "\r\n");

#ifdef STRTOOL_COMPAT
                boost::replace_all(str, "#n", "\r\n");
                if (!str.empty())
#endif
                {
                    os.write(sep.data(), sep.size());
                    os << id << "\r\n" << str << "\r\n";
                }
            }
        }
        ++j;
    }
    os.write(sep.data(), sep.size());
    os << "EOF\r\n";
}

size_t Gbnl::FindDst(uint32_t id, std::vector<StructPtr>& messages,
                     size_t& index) const
{
    auto size = messages.size();
    for (size_t j = 0; j < size; ++j)
    {
        auto j2 = (index+j) % size;
        auto& m = messages[j2];
        size_t k = 0;
        for (size_t i = 0; i < m->GetSize(); ++i)
        {
            auto tid = GetId(*m, i, j2, k);
            if (tid == id)
            {
                index = j2;
                return i;
            }
        }
    }
    return -1;
}

void Gbnl::ReadTxt_(std::istream& is)
{
    std::string line, msg;
    size_t last_index = 0, pos = -1;

    while (is.good())
    {
        std::getline(is, line);

        size_t offs;
        if ((boost::algorithm::starts_with(line, SEP_DASH) && (offs = SEP_DASH.size())) ||
            (boost::algorithm::starts_with(line, SEP_DASH_UTF8) && (offs = SEP_DASH_UTF8.size())))
        {
            if (pos != static_cast<size_t>(-1))
            {
                NEPTOOLS_ASSERT(msg.empty() || msg.back() == '\n');
                if (!msg.empty()) msg.pop_back();
                auto& m = messages[last_index];
                if (m->Is<OffsetString>(pos))
                    m->Get<OffsetString>(pos).str = std::move(msg);
                else
                    strncpy(m->Get<FixStringTag>(pos).str, msg.c_str(),
                            m->GetSize(pos)-1);
                msg.clear();
            }

            if (line.compare(offs, 3, "EOF") == 0)
            {
                RecalcSize();
                return;
            }
            uint32_t id = std::strtoul(line.data() + offs, nullptr, 10);
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

#ifndef NEPTOOLS_WITHOUT_LUA
template <>
struct DynamicStructTypeTraits<Gbnl::OffsetString>
{
    static void Push(Lua::StateRef vm, const void* ptr, size_t size)
    {
        NEPTOOLS_ASSERT(size == sizeof(Gbnl::OffsetString)); (void) size;
        auto ofs = static_cast<const Gbnl::OffsetString*>(ptr);
        if (ofs->offset == static_cast<uint32_t>(-1))
            lua_pushnil(vm);
        else
            vm.Push(ofs->str);
    }

    static void Get(Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        NEPTOOLS_ASSERT(size == sizeof(Gbnl::OffsetString)); (void) size;
        auto ofs = static_cast<Gbnl::OffsetString*>(ptr);

        if (Lua::IsNoneOrNil(lua_type(vm, idx)))
        {
            ofs->offset = static_cast<uint32_t>(-1);
            ofs->str.clear();
        }
        else
        {
            ofs->str = vm.Check<std::string>(idx);
            ofs->offset = 0; // no longer null
        }
    }

    static constexpr bool SIZABLE = false;
    static constexpr const char* NAME = "string";
};

// FixString is zero terminated, Padding is not
template <>
struct DynamicStructTypeTraits<Gbnl::FixStringTag>
{
    static void Push(Lua::StateRef vm, const void* ptr, size_t size)
    {
        auto str = static_cast<const char*>(ptr);
        lua_pushlstring(vm, str, strnlen(str, size));
    }

    static void Get(Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        auto str = vm.Check<StringView>(idx);
        auto dst = static_cast<char*>(ptr);

        auto n = std::min(size-1, str.length());
        memcpy(dst, str.data(), n);
        memset(dst+n, 0, size-n);
    }

    static constexpr bool SIZABLE = true;
    static constexpr const char* NAME = "fix_string";
};

template<>
struct DynamicStructTypeTraits<Gbnl::PaddingTag>
{
    static void Push(Lua::StateRef vm, const void* ptr, size_t size)
    {
        auto str = static_cast<const char*>(ptr);
        lua_pushlstring(vm, str, size);
    }

    static void Get(Lua::StateRef vm, int idx, void* ptr, size_t size)
    {
        auto str = vm.Check<StringView>(idx);
        auto dst = static_cast<char*>(ptr);

        auto n = std::min(size, str.length());
        memcpy(dst, str.data(), n);
        memset(dst+n, 0, size-n);
    }

    static constexpr bool SIZABLE = true;
    static constexpr const char* NAME = "padding";
};
#endif

}

#include "../container/vector.lua.hpp"

NEPTOOLS_DYNAMIC_STRUCT_LUAGEN(
    gbnl, uint8_t, uint16_t, uint32_t, uint64_t, float,
    ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag,
    ::Neptools::Gbnl::PaddingTag);
NEPTOOLS_STD_VECTOR_LUAGEN(gbnl_struct, Neptools::Gbnl::StructPtr);

#include "gbnl.binding.hpp"
