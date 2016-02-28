#include "dumpable.hpp"
#include <fstream>

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

namespace
{
struct DeleteOnExitHelper
{
    ~DeleteOnExitHelper()
    {
        for (auto& p : pths)
            DeleteFileW(p.c_str());
    }

    std::vector<fs::path> pths;
};
void DeleteOnExit(fs::path pth)
{
    static DeleteOnExitHelper hlp;
    hlp.pths.push_back(std::move(pth));
}
}
#endif

void Dumpable::Dump(const fs::path& path) const
{
    auto path2 = path;
    Dump(OpenOut(path2+=".tmp"));

#ifdef WINDOWS
    if (fs::is_regular_file(path))
    {
        auto path3 = path;
        fs::rename(path, path3+=".old");
        if (!DeleteFileW(path3.c_str()) && GetLastError() == ERROR_ACCESS_DENIED)
            DeleteOnExit(std::move(path3));
    }
#endif
    fs::rename(path2, path);
}

void Dumpable::Inspect(const fs::path& path) const
{
    return Inspect(OpenOut(path));
}


std::ostream& operator<<(std::ostream& os, const Dumpable& dmp)
{
    dmp.Inspect(os);
    return os;
}
