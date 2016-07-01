#include "dumpable.hpp"
#include "sink.hpp"
#include "lua/function_call.hpp"
#include "lua/user_type.hpp"
#include <fstream>
#include <boost/filesystem/operations.hpp>

#ifdef WINDOWS
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace
{
struct DeleteOnExitHelper
{
    ~DeleteOnExitHelper()
    {
        for (auto& p : pths)
            DeleteFileW(p.c_str());
    }

    std::vector<boost::filesystem::path> pths;
};
void DeleteOnExit(boost::filesystem::path pth)
{
    static DeleteOnExitHelper hlp;
    hlp.pths.push_back(std::move(pth));
}
}
#endif

namespace Neptools
{

void Dumpable::Dump(const boost::filesystem::path& path) const
{
    auto path2 = path;
    {
        auto sink = Sink::ToFile(path2+=boost::filesystem::unique_path(), GetSize());
        Dump(*sink);
    }

#ifdef WINDOWS
    if (boost::filesystem::is_regular_file(path))
    {
        auto path3 = path;
        boost::filesystem::rename(path, path3+=boost::filesystem::unique_path());
        if (!DeleteFileW(path3.c_str()) && GetLastError() == ERROR_ACCESS_DENIED)
            DeleteOnExit(std::move(path3));
    }
#endif
    boost::filesystem::rename(path2, path);
}

void Dumpable::Inspect(const boost::filesystem::path& path) const
{
    return Inspect(OpenOut(path));
}


std::ostream& operator<<(std::ostream& os, const Dumpable& dmp)
{
    dmp.Inspect(os);
    return os;
}

static std::string InspectToString(const Dumpable& dmp)
{
    std::stringstream ss;
    dmp.Inspect(ss);
    return ss.str();
}

namespace Lua
{
template<>
void TypeRegister::DoRegister<Dumpable>(StateRef, TypeBuilder& bld)
{
#define FT(x) decltype(&x), &x
    bld.Inherit<Dumpable, DynamicObject>()
        .Add<FT(Dumpable::Fixup)>("fixup")
        .Add<FT(Dumpable::GetSize)>("get_size")
        .Add<
            Overload<void (Dumpable::*)(Sink&) const, &Dumpable::Dump>,
            Overload<void (Dumpable::*)(const boost::filesystem::path&) const, &Dumpable::Dump>
        >("dump")
        .Add<
            Overload<void (Dumpable::*)(const boost::filesystem::path&) const, &Dumpable::Inspect>,
            Overload<FT(InspectToString)>
        >("inspect")
        ;
}

static TypeRegister::StateRegister<Dumpable> reg;
}
}
