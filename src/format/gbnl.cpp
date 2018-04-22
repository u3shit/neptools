#include "gbnl_lua.hpp"
#include "../open.hpp"
#include "../sink.hpp"

#include <libshit/except.hpp>
#include <libshit/char_utils.hpp>

#include <map>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <brigand/algorithms/wrap.hpp>

//#define STRTOOL_COMPAT

namespace Neptools
{

  void Gbnl::Header::Validate(size_t chunk_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Gbnl::Header", x)
    VALIDATE(endian == 'L' || endian == 'B');
    VALIDATE(field_04 == 1 && field_06 == 0 && field_08 == 16 && field_0c == 4);
    VALIDATE(descr_offset + msg_descr_size * count_msgs < chunk_size);
    VALIDATE(offset_types + sizeof(TypeDescriptor) * count_types < chunk_size);
    VALIDATE(offset_msgs < chunk_size);
    VALIDATE(field_34 == 0 && field_38 == 0 && field_3c == 0);

    if (memcmp(magic, "GBN", 3) == 0)
      VALIDATE(descr_offset == 0);
    else if (memcmp(magic, "GST", 3) == 0)
      VALIDATE(descr_offset == sizeof(Header));
    else
      VALIDATE(!"Invalid magic");
#undef VALIDATE
  }

  void endian_reverse_inplace(Gbnl::Header& hdr)
  {
    boost::endian::endian_reverse_inplace(hdr.field_04);
    boost::endian::endian_reverse_inplace(hdr.field_06);
    boost::endian::endian_reverse_inplace(hdr.field_08);
    boost::endian::endian_reverse_inplace(hdr.field_0c);
    boost::endian::endian_reverse_inplace(hdr.flags);
    boost::endian::endian_reverse_inplace(hdr.descr_offset);
    boost::endian::endian_reverse_inplace(hdr.count_msgs);
    boost::endian::endian_reverse_inplace(hdr.msg_descr_size);
    boost::endian::endian_reverse_inplace(hdr.count_types);
    boost::endian::endian_reverse_inplace(hdr.offset_types);
    boost::endian::endian_reverse_inplace(hdr.field_28);
    boost::endian::endian_reverse_inplace(hdr.offset_msgs);
    boost::endian::endian_reverse_inplace(hdr.field_30);
    boost::endian::endian_reverse_inplace(hdr.field_34);
    boost::endian::endian_reverse_inplace(hdr.field_38);
    boost::endian::endian_reverse_inplace(hdr.field_3c);
  }

  void endian_reverse_inplace(Gbnl::TypeDescriptor& desc)
  {
    boost::endian::endian_reverse_inplace(desc.type);
    boost::endian::endian_reverse_inplace(desc.offset);
  }

  static size_t GetSize(uint16_t type)
  {
    switch (type)
    {
    case Gbnl::TypeDescriptor::INT8: return 1;
    case Gbnl::TypeDescriptor::INT16: return 2;
    case Gbnl::TypeDescriptor::INT32: return 4;
    case Gbnl::TypeDescriptor::INT64: return 8;
    case Gbnl::TypeDescriptor::FLOAT: return 4;
    case Gbnl::TypeDescriptor::STRING: return 4;
    }
    LIBSHIT_THROW(Libshit::DecodeError, "Gbnl: invalid type");
  }

  Gbnl::Gbnl(Source src)
  {
    ADD_SOURCE(Parse_(src), src);
  }

  void Gbnl::Parse_(Source& src)
  {
#define VALIDATE(msg, x) LIBSHIT_VALIDATE_FIELD("Gbnl" msg, x)

    src.CheckSize(sizeof(Header));
    auto foot = src.PreadGen<Header>(0);
    if (memcmp(foot.magic, "GST", 3) == 0)
      is_gstl = true;
    else
    {
      src.PreadGen(src.GetSize() - sizeof(Header), foot);
      is_gstl = false;
    }

    endian = foot.endian == 'L' ? Endian::LITTLE : Endian::BIG;
    ToNative(foot, endian);
    foot.Validate(src.GetSize());
    flags = foot.flags;
    field_28 = foot.field_28;
    field_30 = foot.field_30;

    src.Seek(foot.offset_types);
    msg_descr_size = foot.msg_descr_size;
    size_t calc_offs = 0;

    Struct::TypeBuilder bld;
    bool int8_in_progress = false;
    for (size_t i = 0; i < foot.count_types; ++i)
    {
      auto type = src.ReadGen<TypeDescriptor>();
      ToNative(type, endian);
      VALIDATE("unordered types", calc_offs <= type.offset);

      Pad(type.offset - calc_offs, bld, int8_in_progress);
      calc_offs = type.offset + ::Neptools::GetSize(type.type);

      switch (type.type)
      {
      case TypeDescriptor::INT8:
        LIBSHIT_ASSERT(!int8_in_progress);
        int8_in_progress = true;
        break;
      case TypeDescriptor::INT16:
        bld.Add<int16_t>();
        break;
      case TypeDescriptor::INT32:
        bld.Add<int32_t>();
        break;
      case TypeDescriptor::INT64:
        bld.Add<int64_t>();
        break;
      case TypeDescriptor::FLOAT:
        bld.Add<float>();
        break;
      case TypeDescriptor::STRING:
        bld.Add<OffsetString>();
        break;
      default:
        LIBSHIT_THROW(Libshit::DecodeError, "GBNL: invalid type");
      }
    }
    Pad(msg_descr_size - calc_offs, bld, int8_in_progress);

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
        case Struct::GetIndexFromType<int8_t>():
          m->Get<int8_t>(i) = src.ReadUint8(endian);
          break;
        case Struct::GetIndexFromType<int16_t>():
          m->Get<int16_t>(i) = src.ReadUint16(endian);
          break;
        case Struct::GetIndexFromType<int32_t>():
          m->Get<int32_t>(i) = src.ReadUint32(endian);
          break;
        case Struct::GetIndexFromType<int64_t>():
          m->Get<int64_t>(i) = src.ReadUint64(endian);
          break;
        case Struct::GetIndexFromType<float>():
        {
          union { float f; uint32_t i; } x;
          x.i = src.ReadUint32(endian);
          m->Get<float>(i) = x.f;
          break;
        }
        case Struct::GetIndexFromType<OffsetString>():
        {
          uint32_t offs = src.ReadUint32(endian);
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

#ifndef LIBSHIT_WITHOUT_LUA
  Gbnl::Gbnl(Libshit::Lua::StateRef vm, Endian endian, bool is_gstl,
             uint32_t flags, uint32_t field_28, uint32_t field_30,
             Libshit::AT<Struct::TypePtr> type, Libshit::Lua::RawTable msgs)
    : endian{endian}, is_gstl{is_gstl}, flags{flags}, field_28{field_28},
      field_30{field_30}, type{std::move(type.Get())}
  {
    auto [len, one] = vm.RawLen01(msgs);
    messages.reserve(len);
    vm.Fori(msgs, one, len, [&](size_t, int type)
    {
      if (type != LUA_TTABLE) vm.TypeError(false, "table", -1);
      messages.emplace_back(brigand::wrap<Struct, DynamicStructLua>::New(
                              vm, this->type, {lua_absindex(vm, -1)}));
    });
  }
#endif

  void Gbnl::Pad(uint16_t diff, Struct::TypeBuilder& bld, bool& int8_in_progress)
  {
    if (int8_in_progress)
    {
      int8_in_progress = false;
      if (diff <= 3) // probably padding
        bld.Add<int8_t>();
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
      WriteDescr(Byte* ptr, Endian e) : ptr{ptr}, e{e} {}
      Byte* ptr;
      Endian e;

      // int8, 16, 32, 64
      template <typename T,
                typename Enable = std::enable_if_t<std::is_integral_v<T>>>
      void operator()(T x, size_t len)
      {
        LIBSHIT_ASSERT(sizeof(T) == len); (void) len;
        *reinterpret_cast<T*>(ptr) = FromNativeCopy(x, e);
        ptr += sizeof(T);
      }

      void operator()(float y, size_t)
      {
        static_assert(sizeof(float) == sizeof(uint32_t));
        union { float f; uint32_t i; } x;
        x.f = y;
        *reinterpret_cast<std::uint32_t*>(ptr) = FromNativeCopy(x.i, e);
        ptr += 4;
      }
      void operator()(const Gbnl::OffsetString& os, size_t)
      {
        *reinterpret_cast<std::uint32_t*>(ptr) = FromNativeCopy(os.offset, e);
        ptr += 4;
      }
      void operator()(const Gbnl::FixStringTag& fs, size_t len)
      {
        strncpy(reinterpret_cast<char*>(ptr), fs.str, len-1);
        ptr[len-1] = '\0';
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
      m->ForEach(WriteDescr{msgd.data(), endian});
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
      case Struct::GetIndexFromType<int8_t>():
        ctrl.type = TypeDescriptor::INT8;
        offs += 1;
        break;
      case Struct::GetIndexFromType<int16_t>():
        ctrl.type = TypeDescriptor::INT16;
        offs += 2;
        break;
      case Struct::GetIndexFromType<int32_t>():
        ctrl.type = TypeDescriptor::INT32;
        offs += 4;
        break;
      case Struct::GetIndexFromType<int64_t>():
        ctrl.type = TypeDescriptor::INT64;
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
        ctrl.type = TypeDescriptor::INT8;
        offs += type->items[i].size;
        break;
      case Struct::GetIndexFromType<PaddingTag>():
        offs += type->items[i].size;
        goto skip;
      }
      FromNative(ctrl, endian);
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

    LIBSHIT_ASSERT(offset == msgs_size);
    auto offset_round = Align(offset);
    sink.Pad(offset_round - offset);

    // sanity checks
    LIBSHIT_ASSERT(msgs_end_round == Align(msg_descr_size * messages.size()));
    LIBSHIT_ASSERT(
      control_end_round == Align(
        msgs_end_round + sizeof(TypeDescriptor) * real_item_count));
    if (!is_gstl) DumpHeader(sink);
  }

  void Gbnl::DumpHeader(Sink& sink) const
  {
    Header head;
    memcpy(head.magic, is_gstl ? "GST" : "GBN", 4);
    head.endian = endian == Endian::LITTLE ? 'L' : 'B';
    head.field_04 = 1;
    head.field_08 = 16;
    head.field_0c = 4;
    head.flags = flags;
    auto offset = is_gstl ? sizeof(Header) : 0;
    head.descr_offset = offset;
    head.count_msgs = messages.size();
    head.msg_descr_size = msg_descr_size;
    head.count_types = real_item_count;
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
    FromNative(head, endian);
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
          os << "nil";
        else
          Libshit::DumpBytes(os, ofs.str);
      }
      void operator()(const Gbnl::FixStringTag& fs, size_t)
      { Libshit::DumpBytes(os, fs.str); }
      void operator()(const Gbnl::PaddingTag& pd, size_t size)
      { Libshit::DumpBytes(os, {pd.pad, size}); }
      void operator()(int8_t x, size_t) { os << static_cast<unsigned>(x); }
      template <typename T> void operator()(T x, size_t) { os << x; }
    };
  }

  void Gbnl::Inspect_(std::ostream& os, unsigned indent) const
  {
    os << "neptools.";
    InspectGbnl(os, indent);
  }
  void Gbnl::InspectGbnl(std::ostream& os, unsigned indent) const
  {
    os << "gbnl(neptools.endian." << ToString(endian) << ", "
       << (is_gstl ? "true" : "false") << ", " << flags << ", " << field_28
       << ", " << field_30 << ", {";

    for (size_t i = 0; i < type->item_count; ++i)
    {
      if (i != 0) os << ", ";
      switch (type->items[i].idx)
      {
      case Struct::GetIndexFromType<int8_t>():       os << "\"int8\"";   break;
      case Struct::GetIndexFromType<int16_t>():      os << "\"int16\"";  break;
      case Struct::GetIndexFromType<int32_t>():      os << "\"int32\"";  break;
      case Struct::GetIndexFromType<int64_t>():      os << "\"int64\"";  break;
      case Struct::GetIndexFromType<float>():        os << "\"float\"";  break;
      case Struct::GetIndexFromType<OffsetString>(): os << "\"string\""; break;
      case Struct::GetIndexFromType<FixStringTag>():
        os << "{\"fix_string\", " << type->items[i].size << "}";
        break;
      case Struct::GetIndexFromType<PaddingTag>():
        os << "{\"padding\", " << type->items[i].size << "}";
        break;
      }
    }

    os << "}, {\n";
    for (const auto& m : messages)
    {
      Indent(os, indent+1) << '{';
      for (size_t i = 0; i < m->GetSize(); ++i)
      {
        if (i != 0) os << ", ";
        m->Visit<void>(i, Print{os});
      }
      os << "},\n";
    }
    Indent(os, indent) << "})";
  }

  void Gbnl::RecalcSize()
  {
    size_t len = 0, count = 0;
    for (size_t i = 0; i < type->item_count; ++i)
      switch (type->items[i].idx)
      {
      case Struct::GetIndexFromType<int8_t>():       len += 1; ++count; break;
      case Struct::GetIndexFromType<int16_t>():      len += 2; ++count; break;
      case Struct::GetIndexFromType<int32_t>():      len += 4; ++count; break;
      case Struct::GetIndexFromType<int64_t>():      len += 8; ++count; break;
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
      LIBSHIT_ASSERT(m->GetType() == type);
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
  static const Libshit::StringView SEP_DASH{
    SEP_DASH_DATA, sizeof(SEP_DASH_DATA)};

  static const char SEP_DASH_UTF8_DATA[] = {
#define REP_MACRO(x,y,z) char(0xe2), char(0x80), char(0x95),
    BOOST_PP_REPEAT(40, REP_MACRO, )
#undef REP_MACRO
    ' '
  };
  static const Libshit::StringView SEP_DASH_UTF8{
    SEP_DASH_UTF8_DATA, sizeof(SEP_DASH_UTF8_DATA)};


  int32_t Gbnl::GetId(const Gbnl::Struct& m, size_t i, size_t j, size_t& k) const
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
    if (!is_gstl && m.Is<int32_t>(0) && (
          (i == 8 && m.GetSize() == 9) || // rebirths
          (i == 105 && m.GetSize() == 107))) // vii
      return m.Get<int32_t>(0);
    else if (is_gstl && m.GetSize() == 3 && m.Is<int32_t>(1))
    {
#ifdef STRTOOL_COMPAT
      if (i == 0) return -1;
#endif
      return m.Get<int32_t>(1) + 100000*(i==0);
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
        if (id != -1)
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

  size_t Gbnl::FindDst(int32_t id, std::vector<StructPtr>& messages,
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
      if ((boost::algorithm::starts_with(line, SEP_DASH) &&
           (offs = SEP_DASH.size())) ||
          (boost::algorithm::starts_with(line, SEP_DASH_UTF8) &&
           (offs = SEP_DASH_UTF8.size())))
      {
        if (pos != static_cast<size_t>(-1))
        {
          LIBSHIT_ASSERT(msg.empty() || msg.back() == '\n');
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
        int32_t id = std::strtol(line.data() + offs, nullptr, 10);
        pos = FindDst(id, messages, last_index);
        if (pos == static_cast<size_t>(-1))
        {
          LIBSHIT_THROW(
            Libshit::DecodeError, "GbnlTxt: invalid id in input",
            "Failed id", id);
        }
      }
      else
      {
        if (pos == static_cast<size_t>(-1))
          LIBSHIT_THROW(
            Libshit::DecodeError, "GbnlTxt: data before separator");
        if (!line.empty() && line.back() == '\r') line.pop_back();
        msg.append(line).append(1, '\n');
      }
    }
    LIBSHIT_THROW(Libshit::DecodeError, "GbnlTxt: EOF");
  }

  static OpenFactory gbnl_open{[](Source src) -> Libshit::SmartPtr<Dumpable>
    {
      if (src.GetSize() < sizeof(Gbnl::Header)) return nullptr;
      char buf[4];
      src.PreadGen(0, buf);
      if (memcmp(buf, "GST", 3) == 0)
        return Libshit::MakeSmart<Gbnl>(src);
      src.PreadGen(src.GetSize() - sizeof(Gbnl::Header), buf);
      if (memcmp(buf, "GBN", 3) == 0)
        return Libshit::MakeSmart<Gbnl>(src);

      return nullptr;
    }};

}

#include "../container/vector.lua.hpp"

NEPTOOLS_DYNAMIC_STRUCT_LUAGEN(
  gbnl, int8_t, int16_t, int32_t, int64_t, float,
  ::Neptools::Gbnl::OffsetString, ::Neptools::Gbnl::FixStringTag,
  ::Neptools::Gbnl::PaddingTag);
NEPTOOLS_STD_VECTOR_LUAGEN(gbnl_struct, Neptools::Gbnl::StructPtr);

#include "gbnl.binding.hpp"
