#ifndef UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#define UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#pragma once

#include <boost/filesystem/path.hpp>
#include "sink.hpp"
#include "refcounted.hpp"

namespace Neptools
{

class Dumpable : public RefCounted
{
public:
    Dumpable() = default;
    Dumpable(const Dumpable&) = delete;
    void operator=(const Dumpable&) = delete;
    virtual ~Dumpable() = default;

    virtual void Fixup() {};
    virtual FilePosition GetSize() const = 0;

    void Dump(Sink& os) const { return Dump_(os); }
    void Dump(Sink&& os) const { return Dump_(os); }
    void Dump(const boost::filesystem::path& path) const;

    void Inspect(std::ostream& os) const { return Inspect_(os); }
    void Inspect(std::ostream&& os) const { return Inspect_(os); }
    void Inspect(const boost::filesystem::path& path) const;

private:
    virtual void Dump_(Sink& sink) const = 0;
    virtual void Inspect_(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Dumpable& dmp);

}
#endif
