#ifndef UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#define UUID_C9446864_0020_4D2F_8E96_CBC6ADCCA3BE
#pragma once

#include "utils.hpp"

#include <libshit/lua/dynamic_object.hpp>
#include <libshit/lua/type_traits.hpp>
#include <libshit/meta.hpp>
#include <libshit/shared_ptr.hpp>

#include <boost/filesystem/path.hpp>

namespace Neptools
{

  class TxtSerializable;
  class Sink;

  class Dumpable : public Libshit::Lua::DynamicObject
  {
    LIBSHIT_LUA_CLASS;
  public:
    Dumpable() = default;
    Dumpable(const Dumpable&) = delete;
    void operator=(const Dumpable&) = delete;
    virtual ~Dumpable() = default;

    virtual void Fixup() {};
    virtual FilePosition GetSize() const = 0;

    LIBSHIT_NOLUA
    virtual Libshit::NotNullSharedPtr<TxtSerializable>
    GetDefaultTxtSerializable(const Libshit::NotNullSharedPtr<Dumpable>& thiz);

    void Dump(Sink& os) const { return Dump_(os); }
    LIBSHIT_NOLUA
    void Dump(Sink&& os) const { return Dump_(os); }
    void Dump(const boost::filesystem::path& path) const;

    LIBSHIT_NOLUA
    void Inspect(std::ostream& os, unsigned indent = 0) const
    { return Inspect_(os, indent); }
    LIBSHIT_NOLUA
    void Inspect(std::ostream&& os, unsigned indent = 0) const
    { return Inspect_(os, indent); }
    void Inspect(const boost::filesystem::path& path) const;
    std::string Inspect() const;

  protected:
    static std::ostream& Indent(std::ostream& os, unsigned indent);

  private:
    virtual void Dump_(Sink& sink) const = 0;
    virtual void Inspect_(std::ostream& os, unsigned indent) const = 0;
  };

  std::ostream& operator<<(std::ostream& os, const Dumpable& dmp);

  inline Libshit::Lua::DynamicObject& GetDynamicObject(Dumpable& d) { return d; }

}
#endif
