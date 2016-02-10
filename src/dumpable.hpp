#ifndef DUMPABLE_HPP
#define DUMPABLE_HPP
#pragma once

#include "utils.hpp"

class Dumpable
{
public:
    Dumpable() = default;
    Dumpable(const Dumpable&) = delete;
    void operator=(const Dumpable&) = delete;
    virtual ~Dumpable() = default;

    virtual void Fixup() = 0;
    void Dump(std::ostream& os) const { return Dump_(os); }
    void Dump(std::ostream&& os) const { return Dump_(os); }
    void Dump(const fs::path& path) const;

    void Inspect(std::ostream& os) const { return Inspect_(os); }
    void Inspect(std::ostream&& os) const { return Inspect_(os); }
    void Inspect(const fs::path& path) const;

private:
    virtual void Dump_(std::ostream& os) const = 0;
    virtual void Inspect_(std::ostream& os) const = 0;
};

std::ostream& operator<<(std::ostream& os, const Dumpable& dmp);

#endif
