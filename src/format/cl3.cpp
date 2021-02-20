#include "cl3.hpp"
#include "stcm/file.hpp"
#include "../open.hpp"

#include <libshit/char_utils.hpp>
#include <libshit/except.hpp>
#include <libshit/container/ordered_map.lua.hpp>
#include <libshit/container/vector.lua.hpp>

#include <fstream>
#include <boost/filesystem/operations.hpp>

namespace Neptools
{

  void Cl3::Header::Validate(FilePosition file_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Cl3::Header", x)
    VALIDATE(memcmp(magic, "CL3", 3) == 0);
    VALIDATE(endian == 'L' || endian == 'B');
    VALIDATE(field_04 == 0);
    VALIDATE(field_08 == 3);
    VALIDATE(sections_offset + sections_count * sizeof(Section) <= file_size);
#undef VALIDATE
  }

  void endian_reverse_inplace(Cl3::Header& hdr) noexcept
  {
    boost::endian::endian_reverse_inplace(hdr.field_04);
    boost::endian::endian_reverse_inplace(hdr.field_08);
    boost::endian::endian_reverse_inplace(hdr.sections_count);
    boost::endian::endian_reverse_inplace(hdr.sections_offset);
    boost::endian::endian_reverse_inplace(hdr.field_14);
  }

  void Cl3::Section::Validate(FilePosition file_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Cl3::Section", x)
    VALIDATE(name.is_valid());
    VALIDATE(data_offset <= file_size);
    VALIDATE(data_offset + data_size <= file_size);
    VALIDATE(field_2c == 0);
    VALIDATE(field_30 == 0 && field_34 == 0 && field_38 == 0 && field_3c == 0);
    VALIDATE(field_40 == 0 && field_44 == 0 && field_48 == 0 && field_4c == 0);
#undef VALIDATE
  }

  void endian_reverse_inplace(Cl3::Section& sec) noexcept
  {
    boost::endian::endian_reverse_inplace(sec.count);
    boost::endian::endian_reverse_inplace(sec.data_size);
    boost::endian::endian_reverse_inplace(sec.data_offset);
    boost::endian::endian_reverse_inplace(sec.field_2c);
    boost::endian::endian_reverse_inplace(sec.field_30);
    boost::endian::endian_reverse_inplace(sec.field_34);
    boost::endian::endian_reverse_inplace(sec.field_38);
    boost::endian::endian_reverse_inplace(sec.field_3c);
    boost::endian::endian_reverse_inplace(sec.field_40);
    boost::endian::endian_reverse_inplace(sec.field_44);
    boost::endian::endian_reverse_inplace(sec.field_48);
    boost::endian::endian_reverse_inplace(sec.field_4c);
  }

  void Cl3::FileEntry::Validate(uint32_t block_size) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Cl3::FileEntry", x)
    VALIDATE(name.is_valid());
    VALIDATE(data_offset <= block_size);
    VALIDATE(data_offset + data_size <= block_size);
    VALIDATE(field_214 == 0 && field_218 == 0 && field_21c == 0);
    VALIDATE(field_220 == 0 && field_224 == 0 && field_228 == 0 && field_22c == 0);
#undef VALIDATE
  }

  void endian_reverse_inplace(Cl3::FileEntry& entry) noexcept
  {
    boost::endian::endian_reverse_inplace(entry.field_200);
    boost::endian::endian_reverse_inplace(entry.data_offset);
    boost::endian::endian_reverse_inplace(entry.data_size);
    boost::endian::endian_reverse_inplace(entry.link_start);
    boost::endian::endian_reverse_inplace(entry.link_count);
    boost::endian::endian_reverse_inplace(entry.field_214);
    boost::endian::endian_reverse_inplace(entry.field_218);
    boost::endian::endian_reverse_inplace(entry.field_21c);
    boost::endian::endian_reverse_inplace(entry.field_220);
    boost::endian::endian_reverse_inplace(entry.field_224);
    boost::endian::endian_reverse_inplace(entry.field_228);
    boost::endian::endian_reverse_inplace(entry.field_22c);
  }

  void Cl3::LinkEntry::Validate(uint32_t i, uint32_t file_count) const
  {
#define VALIDATE(x) LIBSHIT_VALIDATE_FIELD("Cl3::LinkEntry", x)
    VALIDATE(field_00 == 0);
    VALIDATE(linked_file_id < file_count);
    VALIDATE(link_id == i);
    VALIDATE(field_0c == 0);
    VALIDATE(field_10 == 0 && field_14 == 0 && field_18 == 0 && field_1c == 0);
#undef VALIDATE
  }

  void endian_reverse_inplace(Cl3::LinkEntry& entry) noexcept
  {
    boost::endian::endian_reverse_inplace(entry.field_00);
    boost::endian::endian_reverse_inplace(entry.linked_file_id);
    boost::endian::endian_reverse_inplace(entry.link_id);
    boost::endian::endian_reverse_inplace(entry.field_0c);
    boost::endian::endian_reverse_inplace(entry.field_10);
    boost::endian::endian_reverse_inplace(entry.field_14);
    boost::endian::endian_reverse_inplace(entry.field_18);
    boost::endian::endian_reverse_inplace(entry.field_1c);
  }

  void Cl3::Entry::Dispose() noexcept
  {
    links.clear();
    src.reset();
  }

  Cl3::Cl3(Source src)
  {
    ADD_SOURCE(Parse_(src), src);
  }

#if LIBSHIT_WITH_LUA
  Cl3::Cl3(Libshit::Lua::StateRef vm, Endian endian, uint32_t field_14,
           Libshit::Lua::RawTable tbl)
    : endian{endian}, field_14{field_14}
  {
    auto [len, one] = vm.RawLen01(tbl);
    entries.reserve(len);
    bool do_links = false;
    // we need two passes: first add entries, ignoring links
    // then add links when the entries are in place
    // can't do in one pass, as there may be forward links
    vm.Fori(tbl, one, len, [&](size_t, int type)
    {
      if (type == LUA_TTABLE)
      {
        auto [len, one] = vm.RawLen01(-1);
        if (len == 4) // todo: is there a better way??
        {
          do_links = true;
          lua_rawgeti(vm, -1, one); // +1
          lua_rawgeti(vm, -2, one+1); // +2
          lua_rawgeti(vm, -3, one+3); // +3
          entries.emplace_back(
            vm.Get<std::string>(-3),
            vm.Get<uint32_t>(-2),
            vm.Get<Libshit::SmartPtr<Dumpable>>(-1));
          lua_pop(vm, 3);
          return;
        }
      }
      // probably unlikely: try the 3-param ctor, or use existing entry
      entries.push_back(vm.Get<
        Libshit::Lua::AutoTable<Libshit::NotNull<Libshit::SmartPtr<Entry>>>>());
    });

    if (do_links)
    {
      vm.Fori(tbl, one, len, [&](size_t i, int type)
      {
        if (type != LUA_TTABLE) return;
        auto [len, one] = vm.RawLen01(-1);
        if (len != 4) return;
        auto t = lua_rawgeti(vm, -1, one+2); // +1
        if (t != LUA_TTABLE) vm.TypeError(false, "table", -1);

        auto [len2, one2] = vm.RawLen01(-1);
        auto& e = entries[i];
        e.links.reserve(len2);
        vm.Fori(lua_absindex(vm, -1), one2, len2, [&](size_t, int)
        {
          auto it = entries.find(vm.Get<std::string>());
          if (it == entries.end())
            luaL_error(vm, "invalid cl3 link: '%s' not found",
                       vm.Get<const char*>());
          e.links.push_back(&*it);
        });
        lua_pop(vm, 1);
      });
    }
    Fixup();
  }
#endif

  void Cl3::Dispose() noexcept
  {
    entries.clear();
  }

  void Cl3::Parse_(Source& src)
  {
    src.CheckSize(sizeof(Header));
    auto hdr = src.PreadGen<Header>(0);
    endian = hdr.endian == 'L' ? Endian::LITTLE : Endian::BIG;
    ToNative(hdr, endian);
    hdr.Validate(src.GetSize());

    field_14 = hdr.field_14;

    src.Seek(hdr.sections_offset);
    uint32_t secs = hdr.sections_count;

    uint32_t file_offset = 0, file_count = 0, file_size,
      link_offset, link_count = 0;
    for (size_t i = 0; i < secs; ++i)
    {
      auto sec = src.ReadGen<Section>();
      ToNative(sec, endian);
      sec.Validate(src.GetSize());

      if (sec.name == "FILE_COLLECTION")
      {
        file_offset = sec.data_offset;
        file_count = sec.count;
        file_size = sec.data_size;
      }
      else if (sec.name == "FILE_LINK")
      {
        link_offset = sec.data_offset;
        link_count = sec.count;
        LIBSHIT_VALIDATE_FIELD(
          "Cl3::Section",
          sec.data_size == link_count * sizeof(LinkEntry));
      }
    }

    entries.reserve(file_count);
    src.Seek(file_offset);
    for (uint32_t i = 0; i < file_count; ++i)
    {
      auto e = src.ReadGen<FileEntry>();
      ToNative(e, endian);
      e.Validate(file_size);

      entries.emplace_back(
        e.name.c_str(), e.field_200, Libshit::MakeSmart<DumpableSource>(
          src, file_offset+e.data_offset, e.data_size));
    }

    src.Seek(file_offset);
    for (uint32_t i = 0; i < file_count; ++i)
    {
      auto e = src.ReadGen<FileEntry>();
      ToNative(e, endian);
      auto& ls = entries[i].links;
      uint32_t lbase = e.link_start;
      uint32_t lcount = e.link_count;

      for (uint32_t i = lbase; i < lbase+lcount; ++i)
      {
        auto le = src.PreadGen<LinkEntry>(link_offset + i*sizeof(LinkEntry));
        le.Validate(i - lbase, file_count);
        ls.emplace_back(&entries[le.linked_file_id]);
      }
    }
  }

  static constexpr unsigned PAD_BYTES = 0x40;
  static constexpr unsigned PAD = 0x3f;
  void Cl3::Fixup()
  {
    data_size = 0;
    link_count = 0;
    for (auto& e : entries)
    {
      if (e.src)
      {
        e.src->Fixup();
        data_size += e.src->GetSize();
      }
      data_size = (data_size + PAD) & ~PAD;
      link_count += e.links.size();
    }
  }

  FilePosition Cl3::GetSize() const
  {
    FilePosition ret = (sizeof(Header)+PAD) & ~PAD;
    ret = (ret+sizeof(Section)*2+PAD) & ~PAD;
    ret = (ret+sizeof(FileEntry)*entries.size()+PAD) & ~PAD;
    ret += data_size+sizeof(LinkEntry)*link_count;
    return ret;
  }

  Cl3::Entry& Cl3::GetOrCreateFile(std::string_view fname)
  {
    auto it = entries.find(fname, std::less<>{});
    if (it == entries.end())
    {
      entries.emplace_back(std::string{fname});
      return entries.back();
    }
    return *it;
  }

  void Cl3::ExtractTo(const boost::filesystem::path& dir) const
  {
    if (!boost::filesystem::is_directory(dir))
      boost::filesystem::create_directories(dir);

    for (const auto& e : entries)
    {
      if (!e.src) continue;
      auto sink = Sink::ToFile(dir / e.name.c_str(), e.src->GetSize());
      e.src->Dump(*sink);
    }
  }

  void Cl3::UpdateFromDir(const boost::filesystem::path& dir)
  {
    for (auto& e : boost::filesystem::directory_iterator(dir))
      GetOrCreateFile(e.path().filename().string()).src =
        Libshit::MakeSmart<DumpableSource>(Source::FromFile(e));

    for (auto it = entries.begin(); it != entries.end(); )
      if (!boost::filesystem::exists(dir / it->name))
        it = entries.erase(it);
      else
        ++it;
  }

  uint32_t Cl3::IndexOf(const Libshit::WeakSmartPtr<Entry>& ptr) const noexcept
  {
    auto sptr = ptr.lock();
    if (!sptr) return -1;
    auto it = entries.checked_iterator_to(*sptr);
    if (it == entries.end()) return -1;
    return entries.index_of(it);
  }

  void Cl3::Inspect_(std::ostream& os, unsigned indent) const
  {
    os << "neptools.cl3(neptools.endian." << ToString(endian) << ", "
       << field_14 << ", {\n";
    for (auto& e : entries)
    {
      Indent(os, indent+1)
        << '{' << Libshit::Quoted(e.name) << ", " << e.field_200 << ", {";
      bool first = true;
      for (auto& l : e.links)
      {
        if (!first) os << ", ";
        first = false;
        auto ll = l.lock();
        if (ll) Libshit::DumpBytes(os, ll->name);
        else os << "nil";
      }
      os << "}, ";
      if (e.src)
        e.src->Inspect(os, indent+1);
      else
        os << "nil";
      os << "},\n";
    }
    os << "})";
  }

  // workaround, revert to template if/when
  // https://github.com/boostorg/endian/issues/41 is fixed
#define GEN_REVERSE(T)                  \
  static T endian_reverse(T t) noexcept \
  {                                     \
    endian_reverse_inplace(t);          \
    return t;                           \
  }
  GEN_REVERSE(Cl3::Section);
  GEN_REVERSE(Cl3::FileEntry);
  GEN_REVERSE(Cl3::LinkEntry);
#undef GEN_REVERSE

  void Cl3::Dump_(Sink& sink) const
  {
    auto sections_offset = (sizeof(Header)+PAD) & ~PAD;
    auto files_offset = (sections_offset+sizeof(Section)*2+PAD) & ~PAD;
    auto data_offset = (files_offset+sizeof(FileEntry)*entries.size()+PAD) & ~PAD;
    auto link_offset = data_offset + data_size;

    Header hdr;
    memcpy(hdr.magic, "CL3", 3);
    hdr.endian =  endian == Endian::LITTLE ? 'L' : 'B';
    hdr.field_04 = 0;
    hdr.field_08 = 3;
    hdr.sections_count = 2;
    hdr.sections_offset = sections_offset;
    hdr.field_14 = field_14;
    FromNative(hdr, endian);
    sink.WriteGen(hdr);
    sink.Pad(sections_offset-sizeof(Header));

    Section sec;
    memset(&sec, 0, sizeof(Section));
    sec.name = "FILE_COLLECTION";
    sec.count = entries.size();
    sec.data_size = link_offset - files_offset;
    sec.data_offset = files_offset;
    sink.WriteGen(FromNativeCopy(sec, endian));

    sec.name = "FILE_LINK";
    sec.count = link_count;
    sec.data_size = link_count * sizeof(LinkEntry);
    sec.data_offset = link_offset;
    FromNative(hdr, endian);
    sink.WriteGen(sec);
    sink.Pad((PAD_BYTES - ((2*sizeof(Section)) & PAD)) & PAD);

    FileEntry fe;
    fe.field_214 = fe.field_218 = fe.field_21c = 0;
    fe.field_220 = fe.field_224 = fe.field_228 = fe.field_22c = 0;

    // file entry header
    uint32_t offset = data_offset-files_offset, link_i = 0;
    for (auto& e : entries)
    {
      fe.name = e.name;
      fe.field_200 = e.field_200;
      fe.data_offset = offset;
      auto size = e.src ? e.src->GetSize() : 0;
      fe.data_size = size;
      fe.link_start = link_i;
      fe.link_count = e.links.size();
      sink.WriteGen(FromNativeCopy(fe, endian));

      offset = (offset+size+PAD) & ~PAD;
      link_i += e.links.size();
    }
    sink.Pad((PAD_BYTES - ((entries.size()*sizeof(FileEntry)) & PAD)) & PAD);

    // file data
    for (auto& e : entries)
    {
      if (!e.src) continue;
      e.src->Dump(sink);
      sink.Pad((PAD_BYTES - (e.src->GetSize() & PAD)) & PAD);
    }

    // links
    LinkEntry le;
    memset(&le, 0, sizeof(LinkEntry));
    for (auto& e : entries)
    {
      uint32_t i = 0;
      for (const auto& l : e.links)
      {
        le.linked_file_id = IndexOf(l);
        if (le.linked_file_id == uint32_t(-1))
          LIBSHIT_THROW(std::runtime_error, "Invalid file link");
        le.link_id = i++;
        sink.WriteGen(FromNativeCopy(le, endian));
      }
    }
  }

  Stcm::File& Cl3::GetStcm()
  {
    auto dat = entries.find("main.DAT", std::less<>{});
    if (dat == entries.end() || !dat->src)
      LIBSHIT_THROW(Libshit::DecodeError, "Invalid CL3 file: no main.DAT");

    auto stcm = dynamic_cast<Stcm::File*>(dat->src.get());
    if (stcm) return *stcm;

    auto src = Libshit::asserted_cast<DumpableSource*>(dat->src.get());
    auto nstcm = Libshit::MakeSmart<Stcm::File>(src->GetSource());
    auto ret = nstcm.get();
    dat->src = std::move(nstcm);
    return *ret;
  }

  Libshit::NotNullSharedPtr<TxtSerializable> Cl3::GetDefaultTxtSerializable(
      const Libshit::NotNullSharedPtr<Dumpable>& thiz)
  {
    auto& stcm = GetStcm();
    if (!stcm.GetGbnl())
      LIBSHIT_THROW(Libshit::DecodeError, "No GBNL found in STCM");
    return Libshit::NotNullRefCountedPtr<Stcm::File>{&stcm};
  }

  static OpenFactory cl3_open{[](const Source& src) -> Libshit::SmartPtr<Dumpable>
  {
    if (src.GetSize() < sizeof(Cl3::Header)) return nullptr;
    char buf[3];
    src.PreadGen(0, buf);
    if (memcmp(buf, "CL3", 3) == 0)
      return Libshit::MakeSmart<Cl3>(src);
    else
      return nullptr;
  }};

}

LIBSHIT_ORDERED_MAP_LUAGEN(
  cl3_entry, Neptools::Cl3::Entry, Neptools::Cl3::EntryKeyOfValue);
LIBSHIT_STD_VECTOR_LUAGEN(
  cl3_entry, Libshit::WeakRefCountedPtr<Neptools::Cl3::Entry>);
#include "cl3.binding.hpp"
