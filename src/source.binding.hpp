// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.source
template<>
void TypeRegister::DoRegister<::Neptools::Source>(TypeBuilder& bld)
{

    bld.Add<
        decltype(&::Neptools::Lua::TypeTraits<::Neptools::Source>::Make<LuaGetRef<const ::Neptools::Source &>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>), &::Neptools::Lua::TypeTraits<::Neptools::Source>::Make<LuaGetRef<const ::Neptools::Source &>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>
    >("new");
    bld.Add<
        ::Neptools::Source (*)(::boost::filesystem::path), &::Neptools::Source::FromFile
    >("from_file");
    bld.Add<
        void (::Neptools::Source::*)(::Neptools::FilePosition, ::Neptools::FilePosition) noexcept, &::Neptools::Source::Slice<Check::Throw>
    >("slice");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Source::*)() const noexcept, &::Neptools::Source::GetOffset
    >("get_offset");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Source::*)() const noexcept, &::Neptools::Source::GetOrigSize
    >("get_orig_size");
    bld.Add<
        const ::boost::filesystem::path & (::Neptools::Source::*)() const noexcept, &::Neptools::Source::GetFileName
    >("get_file_name");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Source::*)() const noexcept, &::Neptools::Source::GetSize
    >("get_size");
    bld.Add<
        void (::Neptools::Source::*)(::Neptools::FilePosition) noexcept, &::Neptools::Source::Seek<Check::Throw>
    >("seek");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Source::*)() const noexcept, &::Neptools::Source::Tell
    >("tell");
    bld.Add<
        ::Neptools::FilePosition (::Neptools::Source::*)() const noexcept, &::Neptools::Source::GetRemainingSize
    >("get_remaining_size");
    bld.Add<
        bool (::Neptools::Source::*)() const noexcept, &::Neptools::Source::Eof
    >("eof");
    bld.Add<
        void (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::CheckSize
    >("check_size");
    bld.Add<
        void (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::CheckRemainingSize
    >("check_remaining_size");
    bld.Add<
        ::uint8_t (::Neptools::Source::*)(), &::Neptools::Source::ReadLittleUint8<Check::Throw>
    >("read_little_uint8");
    bld.Add<
        ::uint8_t (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::PreadLittleUint8<Check::Throw>
    >("pread_little_uint8");
    bld.Add<
        ::uint16_t (::Neptools::Source::*)(), &::Neptools::Source::ReadLittleUint16<Check::Throw>
    >("read_little_uint16");
    bld.Add<
        ::uint16_t (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::PreadLittleUint16<Check::Throw>
    >("pread_little_uint16");
    bld.Add<
        ::uint32_t (::Neptools::Source::*)(), &::Neptools::Source::ReadLittleUint32<Check::Throw>
    >("read_little_uint32");
    bld.Add<
        ::uint32_t (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::PreadLittleUint32<Check::Throw>
    >("pread_little_uint32");
    bld.Add<
        ::uint64_t (::Neptools::Source::*)(), &::Neptools::Source::ReadLittleUint64<Check::Throw>
    >("read_little_uint64");
    bld.Add<
        ::uint64_t (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::PreadLittleUint64<Check::Throw>
    >("pread_little_uint64");
    bld.Add<
        std::string (::Neptools::Source::*)(), &::Neptools::Source::ReadCString
    >("read_cstring");
    bld.Add<
        std::string (::Neptools::Source::*)(::Neptools::FilePosition) const, &::Neptools::Source::PreadCString
    >("pread_cstring");
    bld.Add<
        void (::Neptools::Source::*)(::Neptools::Sink &) const, &::Neptools::Source::Dump
    >("dump");
    bld.Add<
        std::string (::Neptools::Source::*)() const, &::Neptools::Source::Inspect
    >("inspect");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::Source &, ::Neptools::FileMemSize), &Neptools::LuaRead
    >("read");
    bld.Add<
        ::Neptools::Lua::RetNum (*)(::Neptools::Lua::StateRef, ::Neptools::Source &, ::Neptools::FilePosition, ::Neptools::FileMemSize), &Neptools::LuaPread
    >("pread");

}
static TypeRegister::StateRegister<::Neptools::Source> reg_neptools_source;

}
}


const char ::Neptools::Source::TYPE_NAME[] = "neptools.source";

namespace Neptools
{
namespace Lua
{

// class neptools.dumpable_source
template<>
void TypeRegister::DoRegister<::Neptools::DumpableSource>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::DumpableSource, ::Neptools::Dumpable>();

    bld.Add<
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>), &::Neptools::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>, LuaGetRef<::Neptools::FilePosition>, LuaGetRef<::Neptools::FilePosition>>>,
        Overload<decltype(&::Neptools::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>>), &::Neptools::Lua::TypeTraits<::Neptools::DumpableSource>::Make<LuaGetRef<const ::Neptools::Source &>>>
    >("new");
    bld.Add<
        ::Neptools::Source (::Neptools::DumpableSource::*)() const noexcept, &::Neptools::DumpableSource::GetSource
    >("get_source");

}
static TypeRegister::StateRegister<::Neptools::DumpableSource> reg_neptools_dumpable_source;

}
}


const char ::Neptools::DumpableSource::TYPE_NAME[] = "neptools.dumpable_source";
