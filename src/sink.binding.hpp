// Auto generated code, do not edit. See gen_binding in project root.
#include "lua/user_type.hpp"

namespace Neptools
{
namespace Lua
{

// class neptools.sink
template<>
void TypeRegister::DoRegister<Neptools::Sink>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;
    bld.Inherit<Neptools::Sink, Neptools::Lua::DynamicObject>();

    bld.Add<
        NotNull<RefCountedPtr<Neptools::Sink> > (*)(boost::filesystem::path, FilePosition, bool), &Neptools::Sink::ToFile
    >("to_file");
    bld.Add<
        NotNull<RefCountedPtr<Neptools::Sink> > (*)(), &Neptools::Sink::ToStdOut
    >("to_std_out");
    bld.Add<
        FilePosition (Neptools::Sink::*)() const, &Neptools::Sink::Tell
    >("tell");
    bld.Add<
        void (Neptools::Sink::*)(StringView), &Neptools::Sink::Write<Check::Throw>
    >("write");
    bld.Add<
        void (Neptools::Sink::*)(FileMemSize), &Neptools::Sink::Pad<Check::Throw>
    >("pad");
    bld.Add<
        void (Neptools::Sink::*)(), &Neptools::Sink::Flush
    >("flush");
    bld.Add<
        void (Neptools::Sink::*)(boost::endian::little_uint8_t), &Neptools::Sink::WriteLittleUint8<Check::Throw>
    >("write_little_uint8");
    bld.Add<
        void (Neptools::Sink::*)(boost::endian::little_uint16_t), &Neptools::Sink::WriteLittleUint16<Check::Throw>
    >("write_little_uint16");
    bld.Add<
        void (Neptools::Sink::*)(boost::endian::little_uint32_t), &Neptools::Sink::WriteLittleUint32<Check::Throw>
    >("write_little_uint32");
    bld.Add<
        void (Neptools::Sink::*)(boost::endian::little_uint64_t), &Neptools::Sink::WriteLittleUint64<Check::Throw>
    >("write_little_uint64");
    bld.Add<
        void (Neptools::Sink::*)(NonowningString), &Neptools::Sink::WriteCString<Check::Throw>
    >("write_cstring");
 lua_getfield(vm, -2, "__gc"); bld.SetField("close"); 
}
static TypeRegister::StateRegister<Neptools::Sink> reg_neptools_sink;

}
}

const char Neptools::Sink::TYPE_NAME[] = "neptools.sink";

namespace Neptools
{
namespace Lua
{

// class neptools.memory_sink
template<>
void TypeRegister::DoRegister<Neptools::MemorySink>(StateRef vm, TypeBuilder& bld)
{
    (void) vm;
    bld.Inherit<Neptools::MemorySink, Neptools::Sink>();

    bld.Add<
        Overload<decltype(&::Neptools::MakeSmart<Neptools::MemorySink, LuaGetRef<FileMemSize>>), &::Neptools::MakeSmart<Neptools::MemorySink, LuaGetRef<FileMemSize>>>,
        Overload<NotNull<SmartPtr<Neptools::MemorySink> > (*)(StringView), &MemorySinkFromLua>
    >("new");
    bld.Add<
        StringView (Neptools::MemorySink::*)() const, &Neptools::MemorySink::GetStringView
    >("to_string");

}
static TypeRegister::StateRegister<Neptools::MemorySink> reg_neptools_memory_sink;

}
}

const char Neptools::MemorySink::TYPE_NAME[] = "neptools.memory_sink";

