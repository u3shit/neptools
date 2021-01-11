#ifndef UUID_032F4E0D_CAAE_429A_9928_2ACC24EC51E1
#define UUID_032F4E0D_CAAE_429A_9928_2ACC24EC51E1
#pragma once

#include "../item.hpp"
#include "../../source.hpp"

#include <libshit/check.hpp>
#include <libshit/lua/auto_table.hpp>

#include <boost/endian/arithmetic.hpp>
#include <variant>

namespace Neptools { class RawItem; }

namespace Neptools::Stcm
{

  class InstructionItem final : public ItemWithChildren
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct Header
    {
      boost::endian::little_uint32_t is_call;
      boost::endian::little_uint32_t opcode;
      boost::endian::little_uint32_t param_count;
      boost::endian::little_uint32_t size;

      enum Opcode : uint32_t
      {
        USER_OPCODES = 0x1000,
        SYSTEM_OPCODES_BEGIN = 0xffffff00,
        SYSTEM_OPCODES_END   = 0xffffff14,
      };

      void Validate(FilePosition file_size) const;
    };
    static_assert(sizeof(Header) == 0x10);

    struct Parameter
    {
      struct Type0
      {
        static constexpr uint32_t MEM_OFFSET = 0;
        static constexpr uint32_t UNK = 1;
        static constexpr uint32_t INDIRECT = 2;
        static constexpr uint32_t SPECIAL = 3;
      };

      struct Type0Special
      {
        static constexpr uint32_t READ_STACK_MIN = 0xffffff00;
        static constexpr uint32_t READ_STACK_MAX = 0xffffff0f;
        static constexpr uint32_t READ_4AC_MIN   = 0xffffff20;
        static constexpr uint32_t READ_4AC_MAX   = 0xffffff27;
        static constexpr uint32_t INSTR_PTR0     = 0xffffff40;
        static constexpr uint32_t INSTR_PTR1     = 0xffffff41;
        static constexpr uint32_t COLL_LINK      = 0xffffff42;
        static constexpr uint32_t EXPANSION      = 0xffffff43;
      };

      struct Type48
      {
        static constexpr uint32_t MEM_OFFSET = 0;
        static constexpr uint32_t IMMEDIATE = 1;
        static constexpr uint32_t INDIRECT = 2;
        static constexpr uint32_t SPECIAL = 3;
      };

      struct Type48Special
      {
        static constexpr uint32_t READ_STACK_MIN = 0xffffff00;
        static constexpr uint32_t READ_STACK_MAX = 0xffffff0f;
        static constexpr uint32_t READ_4AC_MIN   = 0xffffff20;
        static constexpr uint32_t READ_4AC_MAX   = 0xffffff27;
      };

      boost::endian::little_uint32_t param_0;
      boost::endian::little_uint32_t param_4;
      boost::endian::little_uint32_t param_8;

      static constexpr inline uint32_t TypeTag(uint32_t x) { return x >> 30; }
      static constexpr inline uint32_t Value(uint32_t x) { return x & 0x3fffffff; }
      static constexpr inline uint32_t Tag(uint32_t tag, uint32_t val)
      { return (tag << 30) | val; }
      void Validate(FilePosition file_size) const;
    };
    static_assert(sizeof(Parameter) == 0xc);

    class Param;
    InstructionItem(Key k, Context& ctx) : ItemWithChildren{k, ctx} {}
    InstructionItem(Key k, Context& ctx, Source src);
    InstructionItem(Key k, Context& ctx, Libshit::NotNull<LabelPtr> tgt)
      : ItemWithChildren{k, ctx}, opcode_target{std::move(tgt)} {}
    InstructionItem(Key k, Context& ctx, Libshit::NotNull<LabelPtr> tgt,
                    Libshit::AT<std::vector<Param>> params)
      : ItemWithChildren{k, ctx}, params{std::move(params.Get())},
        opcode_target{std::move(tgt)} {}

    InstructionItem(Key k, Context& ctx, uint32_t opcode)
      : ItemWithChildren{k, ctx}, opcode_target{opcode} {}
    InstructionItem(Key k, Context& ctx, uint32_t opcode,
                    Libshit::AT<std::vector<Param>> params)
      : ItemWithChildren{k, ctx}, params{std::move(params.Get())},
        opcode_target{opcode} {}
    static InstructionItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override;
    void Fixup() override;
    void Dispose() noexcept override;

    bool IsCall() const noexcept
    { return !std::holds_alternative<uint32_t>(opcode_target); }

    uint32_t GetOpcode() const { return std::get<0>(opcode_target); }

    void SetOpcode(uint32_t oc) noexcept { opcode_target = oc; }
    Libshit::NotNull<LabelPtr> GetTarget() const
    { return std::get<1>(opcode_target); }

    void SetTarget(Libshit::NotNull<LabelPtr> label) noexcept
    { opcode_target = label; }

    class Param48 : public Libshit::Lua::ValueObject
    {
      LIBSHIT_LUA_CLASS;
    public:
      enum class LIBSHIT_LUAGEN() Type
      {
#define NEPTOOLS_GEN_TYPES(x, ...) \
        x(MEM_OFFSET, __VA_ARGS__) \
        x(IMMEDIATE, __VA_ARGS__)  \
        x(INDIRECT, __VA_ARGS__)   \
        x(READ_STACK, __VA_ARGS__) \
        x(READ_4AC, __VA_ARGS__)
#define NEPTOOLS_GEN_ENUM(x,y) x,
        NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_ENUM,)
      };
      using Variant = std::variant<
        Libshit::NotNull<LabelPtr>, uint32_t, uint32_t, uint32_t, uint32_t>;

      Param48(Context& ctx, uint32_t val) : val{GetVariant(ctx, val)} {}
      template <size_t type, typename T> LIBSHIT_NOLUA
      Param48(std::in_place_index_t<type> x, T val) : val{x, std::move(val)} {}

      uint32_t Dump() const noexcept;

      Type GetType() const noexcept { return static_cast<Type>(val.index()); }

      template <typename Visitor> LIBSHIT_NOLUA
      auto Visit(Visitor&& v) const
      { return std::visit(std::forward<Visitor>(v), val); }

#define NEPTOOLS_GEN_TMPL(x,xname) LIBSHIT_LUAGEN(      \
        name=xname..string.lower(#x), template_params={ \
          "Neptools::Stcm::InstructionItem::Param48::Type::"..#x})
      template <Type type> NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_TMPL, "get_")
      auto Get() const { return std::get<static_cast<size_t>(type)>(val); }
      template <Type type> LIBSHIT_NOLUA
      void Set(std::variant_alternative_t<
                 static_cast<size_t>(type), Variant> nval)
      { val.emplace(std::in_place_index<type>(std::move(nval))); }

      template <Type type> NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_TMPL, "new_")
      static Param48 New(std::variant_alternative_t<
                           static_cast<size_t>(type), Variant> nval)
      { return {std::in_place_index<static_cast<size_t>(type)>, std::move(nval)}; }
#undef NEPTOOLS_GEN_TMPL
#undef NEPTOOLS_GEN_TYPES


      // NEPTOOLS_GEN_INT(Immediate, IMMEDIATE,  Param48, Parameter::TypeTag(val) == 0)
      // NEPTOOLS_GEN_INT(Indirect,  INDIRECT,   Param48, Parameter::TypeTag(val) == 0)
      // NEPTOOLS_GEN_INT(ReadStack, READ_STACK, Param48, val < 16)
      // NEPTOOLS_GEN_INT(Read4ac,   READ_4AC,   Param48, val < 16)

    private:
      Variant val;
      static Variant GetVariant(Context& ctx, uint32_t val);
    } LIBSHIT_LUAGEN(post_register="bld.TaggedNew();");

    class Param : public Libshit::Lua::ValueObject
    {
      LIBSHIT_LUA_CLASS;
    public:
      struct MemOffset : Libshit::Lua::ValueObject
      {
        Libshit::NotNull<LabelPtr> target;
        Param48 param_4;
        Param48 param_8;

        MemOffset(Libshit::NotNull<LabelPtr> target, Param48 param_4,
                  Param48 param_8)
          : target{std::move(target)}, param_4{std::move(param_4)},
            param_8{std::move(param_8)} {}
        LIBSHIT_LUA_CLASS;
      };
      struct Indirect : Libshit::Lua::ValueObject
      {
        uint32_t param_0;
        Param48 param_8;

        Indirect(uint32_t param_0, Param48 param_8)
          : param_0{param_0}, param_8{std::move(param_8)} {}
        LIBSHIT_LUA_CLASS;
      };
      enum class LIBSHIT_LUAGEN() Type
      {
#define NEPTOOLS_GEN_TYPES(x, ...) \
        x(MEM_OFFSET, __VA_ARGS__) \
        x(INDIRECT, __VA_ARGS__)   \
        x(READ_STACK, __VA_ARGS__) \
        x(READ_4AC, __VA_ARGS__)   \
        x(INSTR_PTR0, __VA_ARGS__) \
        x(INSTR_PTR1, __VA_ARGS__) \
        x(COLL_LINK, __VA_ARGS__)  \
        x(EXPANSION, __VA_ARGS__)
        NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_ENUM,)
#undef NEPTOOLS_GEN_ENUM
      };
      using Variant = std::variant<
        MemOffset, Indirect, uint32_t, uint32_t, Libshit::NotNull<LabelPtr>,
        Libshit::NotNull<LabelPtr>, Libshit::NotNull<LabelPtr>,
        Libshit::NotNull<LabelPtr>>;

      LIBSHIT_NOLUA
      Param(Context& ctx, const Parameter& p) : val{GetVariant(ctx, p)} {}
      template <size_t type, typename T> LIBSHIT_NOLUA
      Param(std::in_place_index_t<type> x, T val) : val{x, std::move(val)} {}

      void Dump(Sink& sink) const;
      LIBSHIT_NOLUA void Dump(Sink&& sink) const { Dump(sink); }

      Type GetType() const noexcept { return static_cast<Type>(val.index()); }

      template <typename Visitor> LIBSHIT_NOLUA
      auto Visit(Visitor&& v) const
      { return std::visit(std::forward<Visitor>(v), val); }

#define NEPTOOLS_GEN_TMPL(x,xname) LIBSHIT_LUAGEN(      \
        name=xname..string.lower(#x), template_params={ \
          "Neptools::Stcm::InstructionItem::Param::Type::"..#x})
      template <Type type> NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_TMPL, "get_")
        auto Get() const { return std::get<static_cast<size_t>(type)>(val); }
      template <Type type> LIBSHIT_NOLUA
      void Set(std::variant_alternative_t<
                 static_cast<size_t>(type), Variant> nval)
      { val.emplace(std::in_place_index<type>(std::move(nval))); }

      template <Type type> NEPTOOLS_GEN_TYPES(NEPTOOLS_GEN_TMPL, "new_")
      static Param New(std::variant_alternative_t<
                         static_cast<size_t>(type), Variant> nval)
      { return {std::in_place_index<static_cast<size_t>(type)>, std::move(nval)}; }

      static Param NewMemOffset(
        Libshit::NotNull<LabelPtr> target, Libshit::AT<Param48> param_4,
        Libshit::AT<Param48> param_8)
      {
        return New<Type::MEM_OFFSET>({
            std::move(target), std::move(param_4.Get()),
              std::move(param_8.Get())});
      }
      static Param NewIndirect(uint32_t param_0, Libshit::AT<Param48> param_8)
      { return New<Type::INDIRECT>({param_0, std::move(param_8.Get())}); }

#undef NEPTOOLS_GEN_TYPES
#undef NEPTOOL_GEN_ENUM
#undef NEPTOOLS_GEN_TMPL

      // NEPTOOLS_GEN_LABEL(MemOffset, MEM_OFFSET, Param)
      // NEPTOOLS_GEN_INT  (Indirect,  INDIRECT,   Param, Parameter::TypeTag(val) == 0)
      // NEPTOOLS_GEN_INT  (ReadStack, READ_STACK, Param, val < 16)
      // NEPTOOLS_GEN_INT  (Read4ac,   READ_4AC,   Param, val < 16)
      // NEPTOOLS_GEN_LABEL(InstrPtr0, INSTR_PTR0, Param)
      // NEPTOOLS_GEN_LABEL(InstrPtr1, INSTR_PTR1, Param)
      // NEPTOOLS_GEN_LABEL(CollLink,  COLL_LINK,  Param)

    private:
      Variant val;
      static Variant GetVariant(Context& ctx, const Parameter& in);
    } LIBSHIT_LUAGEN(post_register="bld.TaggedNew();");

    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    std::vector<Param> params;

  private:
    std::variant<uint32_t, Libshit::NotNull<LabelPtr>> opcode_target;

    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void Parse_(Context& ctx, Source& src);
  };

  std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p);
  std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p);

}

LIBSHIT_ENUM(Neptools::Stcm::InstructionItem::Param48::Type);
LIBSHIT_ENUM(Neptools::Stcm::InstructionItem::Param::Type);

#endif
