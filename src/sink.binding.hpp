// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Sink::TYPE_NAME[] = "neptools.sink";

const char ::Neptools::MemorySink::TYPE_NAME[] = "neptools.memory_sink";

namespace Libshit::Lua
{

  // class neptools.sink
  template<>
  void TypeRegisterTraits<::Neptools::Sink>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      static_cast<::Libshit::NotNull<Libshit::RefCountedPtr<::Neptools::Sink> > (*)(::boost::filesystem::path, ::Neptools::FilePosition, bool)>(::Neptools::Sink::ToFile)
    >("to_file");
    bld.AddFunction<
      static_cast<::Libshit::NotNull<Libshit::RefCountedPtr<::Neptools::Sink> > (*)()>(::Neptools::Sink::ToStdOut)
    >("to_std_out");
    bld.AddFunction<
      static_cast<::Neptools::FilePosition (::Neptools::Sink::*)() const noexcept>(&::Neptools::Sink::Tell)
    >("tell");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::Libshit::StringView)>(&::Neptools::Sink::Write<Check::Throw>)
    >("write");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::Neptools::FileMemSize)>(&::Neptools::Sink::Pad<Check::Throw>)
    >("pad");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)()>(&::Neptools::Sink::Flush)
    >("flush");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::boost::endian::little_uint8_t)>(&::Neptools::Sink::WriteLittleUint8<Check::Throw>)
    >("write_little_uint8");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::boost::endian::little_uint16_t)>(&::Neptools::Sink::WriteLittleUint16<Check::Throw>)
    >("write_little_uint16");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::boost::endian::little_uint32_t)>(&::Neptools::Sink::WriteLittleUint32<Check::Throw>)
    >("write_little_uint32");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::boost::endian::little_uint64_t)>(&::Neptools::Sink::WriteLittleUint64<Check::Throw>)
    >("write_little_uint64");
    bld.AddFunction<
      static_cast<void (::Neptools::Sink::*)(::Libshit::NonowningString)>(&::Neptools::Sink::WriteCString<Check::Throw>)
    >("write_cstring");
 lua_getfield(bld, -2, "__gc"); bld.SetField("close");
  }
  static TypeRegister::StateRegister<::Neptools::Sink> reg_neptools_sink;

  // class neptools.memory_sink
  template<>
  void TypeRegisterTraits<::Neptools::MemorySink>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::MemorySink, ::Neptools::Sink>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::MemorySink>::Make<LuaGetRef<::Neptools::FileMemSize>>,
      static_cast<::Libshit::NotNull<Libshit::SmartPtr<::Neptools::MemorySink> > (*)(::Libshit::StringView)>(&Neptools::MemorySinkFromLua)
    >("new");
    bld.AddFunction<
      static_cast<::Libshit::StringView (::Neptools::MemorySink::*)() const noexcept>(&::Neptools::MemorySink::GetStringView)
    >("to_string");

  }
  static TypeRegister::StateRegister<::Neptools::MemorySink> reg_neptools_memory_sink;

}
#endif
