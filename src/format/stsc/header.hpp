#ifndef UUID_F87DF453_742A_4C38_8660_ABC81ACB04B8
#define UUID_F87DF453_742A_4C38_8660_ABC81ACB04B8
#pragma once

#include "file.hpp"
#include "../../source.hpp"
#include "../item.hpp"

#include <boost/endian/arithmetic.hpp>

#include <string_view>

namespace Neptools::Stsc
{

  class HeaderItem final : public Item
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct Header
    {
      char magic[4];
      boost::endian::little_uint32_t entry_point;
      boost::endian::little_uint32_t flags;

      void Validate(FilePosition size) const;
    };
    static_assert(sizeof(Header) == 12);

    struct ExtraHeader2Ser
    {
      boost::endian::little_uint16_t field_0;
      boost::endian::little_uint16_t field_2;
      boost::endian::little_uint16_t field_4;
      boost::endian::little_uint16_t field_6;
      boost::endian::little_uint16_t field_8;
      boost::endian::little_uint16_t field_a;
      boost::endian::little_uint16_t field_c;
    };
    static_assert(sizeof(ExtraHeader2Ser) == 14);

    class LIBSHIT_LUAGEN(name="extra_headers_2") ExtraHeaders2
      : public Libshit::Lua::ValueObject
    {
      LIBSHIT_LUA_CLASS;

    public:
      std::uint16_t field_0;
      std::uint16_t field_2;
      std::uint16_t field_4;
      std::uint16_t field_6;
      std::uint16_t field_8;
      std::uint16_t field_a;
      std::uint16_t field_c;

      ExtraHeaders2(
        std::uint16_t field_0, std::uint16_t field_2, std::uint16_t field_4,
        std::uint16_t field_6, std::uint16_t field_8, std::uint16_t field_a,
        std::uint16_t field_c) noexcept
        : field_0{field_0}, field_2{field_2}, field_4{field_4},
          field_6{field_6}, field_8{field_8}, field_a{field_a},
          field_c{field_c} {}
    };

    HeaderItem(Key k, Context& ctx, Source src);
    HeaderItem(
      Key k, Context& ctx, Libshit::NotNull<LabelPtr> entry_point,
      std::optional<std::string_view> extra_headers_1,
      std::optional<ExtraHeaders2> extra_headers_2,
      std::optional<uint16_t> extra_headers_4);
    static HeaderItem& CreateAndInsert(ItemPointer ptr, Flavor flavor);

    FilePosition GetSize() const noexcept override;

    Libshit::NotNull<LabelPtr> entry_point;

    std::optional<std::array<std::uint8_t, 32>> extra_headers_1;
    std::optional<ExtraHeaders2> extra_headers_2;
    std::optional<std::uint16_t> extra_headers_4;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
  };

}

#endif
