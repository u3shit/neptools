// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Source::TYPE_NAME[] = "neptools.source";

const char ::Neptools::DumpableSource::TYPE_NAME[] = "neptools.dumpable_source";

namespace Libshit::Lua
{

  // class neptools.source
  template<>
  void TypeRegisterTraits<::Neptools::Source>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Source>::Make<LuaGetRef<::Neptools::Source>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Source (*)(const ::boost::filesystem::path &)>(::Neptools::Source::FromFile)
    >("from_file");
    bld.AddFunction<
      static_cast<::Neptools::Source (*)(::boost::filesystem::path, int, bool)>(::Neptools::Source::FromFd)
    >("from_fd");
    bld.AddFunction<
      static_cast<::Neptools::Source (*)(std::string)>(::Neptools::Source::FromMemory),
      static_cast<::Neptools::Source (*)(::boost::filesystem::path, std::string)>(::Neptools::Source::FromMemory)
    >("from_memory");
    bld.AddFunction<
      static_cast<void (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::FilePosition) noexcept>(&::Neptools::Source::Slice<Check::Throw>)
    >("slice");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::GetOffset)
    >("get_offset");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::GetOrigSize)
    >("get_orig_size");
    bld.AddFunction<
      static_cast<const ::boost::filesystem::path & (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::GetFileName)
    >("get_file_name");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::GetSize)
    >("get_size");
    bld.AddFunction<
      static_cast<void (::Neptools::Source::*)(::Neptools::FilePosition) noexcept>(&::Neptools::Source::Seek<Check::Throw>)
    >("seek");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::Tell)
    >("tell");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::GetRemainingSize)
    >("get_remaining_size");
    bld.AddFunction<
      static_cast<bool (::Neptools::Source::*)() const noexcept>(&::Neptools::Source::Eof)
    >("eof");
    bld.AddFunction<
      static_cast<void (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::CheckSize)
    >("check_size");
    bld.AddFunction<
      static_cast<void (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::CheckRemainingSize)
    >("check_remaining_size");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)(::Neptools::Endian)>(&::Neptools::Source::ReadUint8<Check::Throw>)
    >("read_uint8");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::Endian)>(&::Neptools::Source::PreadUint8<Check::Throw>)
    >("pread_uint8");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadLittleUint8<Check::Throw>)
    >("read_little_uint8");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadLittleUint8<Check::Throw>)
    >("pread_little_uint8");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadBigUint8<Check::Throw>)
    >("read_big_uint8");
    bld.AddFunction<
      static_cast<std::uint8_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadBigUint8<Check::Throw>)
    >("pread_big_uint8");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)(::Neptools::Endian)>(&::Neptools::Source::ReadUint16<Check::Throw>)
    >("read_uint16");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::Endian)>(&::Neptools::Source::PreadUint16<Check::Throw>)
    >("pread_uint16");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadLittleUint16<Check::Throw>)
    >("read_little_uint16");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadLittleUint16<Check::Throw>)
    >("pread_little_uint16");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadBigUint16<Check::Throw>)
    >("read_big_uint16");
    bld.AddFunction<
      static_cast<std::uint16_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadBigUint16<Check::Throw>)
    >("pread_big_uint16");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)(::Neptools::Endian)>(&::Neptools::Source::ReadUint32<Check::Throw>)
    >("read_uint32");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::Endian)>(&::Neptools::Source::PreadUint32<Check::Throw>)
    >("pread_uint32");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadLittleUint32<Check::Throw>)
    >("read_little_uint32");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadLittleUint32<Check::Throw>)
    >("pread_little_uint32");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadBigUint32<Check::Throw>)
    >("read_big_uint32");
    bld.AddFunction<
      static_cast<std::uint32_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadBigUint32<Check::Throw>)
    >("pread_big_uint32");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)(::Neptools::Endian)>(&::Neptools::Source::ReadUint64<Check::Throw>)
    >("read_uint64");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::Endian)>(&::Neptools::Source::PreadUint64<Check::Throw>)
    >("pread_uint64");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadLittleUint64<Check::Throw>)
    >("read_little_uint64");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadLittleUint64<Check::Throw>)
    >("pread_little_uint64");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)()>(&::Neptools::Source::ReadBigUint64<Check::Throw>)
    >("read_big_uint64");
    bld.AddFunction<
      static_cast<std::uint64_t (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadBigUint64<Check::Throw>)
    >("pread_big_uint64");
    bld.AddFunction<
      static_cast<std::string (::Neptools::Source::*)()>(&::Neptools::Source::ReadCString)
    >("read_cstring");
    bld.AddFunction<
      static_cast<std::string (::Neptools::Source::*)(::Neptools::FilePosition) const>(&::Neptools::Source::PreadCString)
    >("pread_cstring");
    bld.AddFunction<
      static_cast<void (::Neptools::Source::*)(::Neptools::Sink &) const>(&::Neptools::Source::Dump)
    >("dump");
    bld.AddFunction<
      static_cast<std::string (::Neptools::Source::*)() const>(&::Neptools::Source::Inspect)
    >("inspect");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::Neptools::Source &, ::Neptools::FileMemSize)>(&Neptools::LuaRead)
    >("read");
    bld.AddFunction<
      static_cast<::Libshit::Lua::RetNum (*)(::Libshit::Lua::StateRef, ::Neptools::Source &, ::Neptools::FilePosition, ::Neptools::FileMemSize)>(&Neptools::LuaPread)
    >("pread");

  }
  static TypeRegister::StateRegister<::Neptools::Source> reg_neptools_source;

  // class neptools.dumpable_source
  template<>
  void TypeRegisterTraits<::Neptools::DumpableSource>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::DumpableSource, ::Neptools::Dumpable>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>,
      &::Libshit::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Source (::Neptools::DumpableSource::*)() const noexcept>(&::Neptools::DumpableSource::GetSource)
    >("get_source");

  }
  static TypeRegister::StateRegister<::Neptools::DumpableSource> reg_neptools_dumpable_source;

}
#endif
