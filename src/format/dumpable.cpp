#include "dumpable.hpp"
#include <fstream>

void Dumpable::Dump(const fs::path& path) const
{
    auto path2 = path;
    Dump(OpenOut(path2+=".tmp"));
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
