#ifndef UUID_9037C300_D4EF_473C_8387_A1A9797069A7
#define UUID_9037C300_D4EF_473C_8387_A1A9797069A7
#pragma once

#include "../dumpable.hpp"
#include "../source.hpp"
#include "../dynamic_struct.hpp"
#include "../txt_serializable.hpp"

#include <libshit/lua/auto_table.hpp>
#include <boost/endian/arithmetic.hpp>
#include <vector>

namespace Neptools
{

  class Gbnl : public Dumpable, public TxtSerializable
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct Header // or Footer
    {
      char magic[3];
      char version;

      boost::endian::little_uint32_t field_04;
      boost::endian::little_uint32_t field_08;
      boost::endian::little_uint32_t field_0c;
      boost::endian::little_uint32_t flags; // 1 if there's a string, 0 otherwise?
      boost::endian::little_uint32_t descr_offset;
      boost::endian::little_uint32_t count_msgs;
      boost::endian::little_uint32_t msg_descr_size;
      boost::endian::little_uint16_t count_types;
      boost::endian::little_uint16_t field_22;
      boost::endian::little_uint32_t offset_types;
      boost::endian::little_uint32_t field_28;
      boost::endian::little_uint32_t offset_msgs;
      boost::endian::little_uint32_t field_30;
      boost::endian::little_uint32_t field_34;
      boost::endian::little_uint32_t field_38;
      boost::endian::little_uint32_t field_3c;

      void Validate(size_t chunk_size) const;
    };
    static_assert(sizeof(Header) == 0x40);

    struct TypeDescriptor
    {
      enum Type
      {
        INT32  = 0,
        INT8   = 1,
        INT16  = 2,
        FLOAT  = 3,
        STRING = 5,
        INT64  = 6,
      };
      boost::endian::little_uint16_t type;
      boost::endian::little_uint16_t offset;
    };
    static_assert(sizeof(TypeDescriptor) == 0x04);

    struct OffsetString
    {
      std::string str;
      uint32_t offset;
    };

    struct FixStringTag { char str[1]; };
    struct PaddingTag { char pad[1]; };

    using Struct = DynamicStruct<
      int8_t, int16_t, int32_t, int64_t, float, OffsetString,
      FixStringTag, PaddingTag>;
    using StructPtr = Libshit::NotNull<boost::intrusive_ptr<Struct>>;
    using Messages = std::vector<StructPtr>;

    Gbnl(Source src);
    Gbnl(bool is_gstl, uint32_t flags, uint32_t field_28, uint32_t field_30,
         Libshit::AT<Struct::TypePtr> type)
      : is_gstl{is_gstl}, flags{flags}, field_28{field_28}, field_30{field_30},
        type{std::move(type.Get())} {}
#ifndef LIBSHIT_WITHOUT_LUA
    Gbnl(Libshit::Lua::StateRef vm, bool is_gstl, uint32_t flags,
         uint32_t field_28, uint32_t field_30, Libshit::AT<Struct::TypePtr> type,
         Libshit::Lua::RawTable messages);
#endif

    void Fixup() override { RecalcSize(); }

    bool is_gstl;
    uint32_t flags, field_28, field_30;

    // no setter - it doesn't work how you expect in lua
    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    Messages messages;

    Struct::TypePtr type;

    void RecalcSize();
    FilePosition GetSize() const noexcept override;

    using FailedId = boost::error_info<struct FailedIdTag, int32_t>;

  protected:
    // todo: private after removing GbnlItem
    void Dump_(Sink& sink) const override;
    void InspectGbnl(std::ostream& os, unsigned indent) const;
    void Inspect_(std::ostream& os, unsigned indent) const override;

  private:
    void WriteTxt_(std::ostream& os) const override;
    void ReadTxt_(std::istream& is) override;

    void Parse_(Source& src);
    void DumpHeader(Sink& sink) const;
    void Pad(uint16_t diff, Struct::TypeBuilder& bld, bool& int8_in_progress);
    FilePosition Align(FilePosition x) const noexcept;

    int32_t GetId(const Gbnl::Struct& m, size_t i, size_t j, size_t& k) const;
    size_t FindDst(int32_t id, std::vector<StructPtr>& messages,
                   size_t& index) const;

    size_t msg_descr_size, msgs_size;
    size_t real_item_count; // excluding dummy pad items
  };

}
#endif
