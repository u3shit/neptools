#ifndef UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#define UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#pragma once

#include "file.hpp"
#include "../../source.hpp"
#include "../item.hpp"

#include <libshit/lua/auto_table.hpp>

#include <boost/endian/arithmetic.hpp>

namespace Neptools::Stsc
{

  class InstructionBase : public Item
  {
    LIBSHIT_LUA_CLASS;
  public:
    InstructionBase(Key k, Context& ctx, uint8_t opcode)
      : Item{k, ctx}, opcode{opcode} {}

    static InstructionBase& CreateAndInsert(ItemPointer ptr, Flavor f);

    const uint8_t opcode;

  protected:
    void InstrDump(Sink& sink) const;
    std::ostream& InstrInspect(std::ostream& os, unsigned indent) const;

  private:
    virtual void PostInsert(Flavor f) = 0;
  };

  using Tagged = uint32_t;
  struct Code;

  template <typename T> struct TupleTypeMap { using Type = T; };
  template<> struct TupleTypeMap<std::string> { using Type = LabelPtr; };
  template<> struct TupleTypeMap<void*> { using Type = LabelPtr; };
  template<> struct TupleTypeMap<Code*> { using Type = LabelPtr; };

  template <typename T>
  using TupleTypeMapT = typename TupleTypeMap<T>::Type;

  // namegen
  template <char... Args> struct StringContainer
  {
    template <char... X> using Append = StringContainer<Args..., X...>;
    static inline constexpr const char str[sizeof...(Args)+1] = { Args... };
  };

  template <typename Cnt, typename... Args> struct AppendTypes;
  template <typename Cnt> struct AppendTypes<Cnt> { using Type = Cnt; };
#define NEPTOOLS_GEN(x, ...)                                         \
  template <typename Cnt, typename... Args>                          \
  struct AppendTypes<Cnt, x, Args...>                                \
  {                                                                  \
    using Type = typename AppendTypes<typename Cnt::template Append< \
      '_',__VA_ARGS__>, Args...>::Type;                              \
  }
  NEPTOOLS_GEN(uint8_t, 'u','i','n','t','8');
  NEPTOOLS_GEN(uint16_t, 'u','i','n','t','1','6');
  NEPTOOLS_GEN(uint32_t, 'u','i','n','t','3','2');
  NEPTOOLS_GEN(float, 'f','l','o','a','t');
  NEPTOOLS_GEN(std::string, 's','t','r','i','n','g');
  NEPTOOLS_GEN(Code*, 'c','o','d','e');
  NEPTOOLS_GEN(void*, 'd','a','t','a');
#undef NEPTOOLS_GEN

  template <bool NoReturn, typename... Args>
  class SimpleInstruction final : public InstructionBase
  {
    using Pref = StringContainer<
      'n','e','p','t','o','o','l','s','.','s','t','s','c','.',
      's','i','m','p','l','e','_','i','n','s','t','r','u','c','t','i','o','n','_'>;
    using Boold = std::conditional_t<
      NoReturn,
      Pref::Append<'t','r','u','e'>,
      Pref::Append<'f','a','l','s','e'>>;
    using TypeName = typename AppendTypes<Boold, Args...>::Type;

    LIBSHIT_DYNAMIC_OBJ_GEN;
  public:
    static constexpr const char* TYPE_NAME = TypeName::str;

    SimpleInstruction(Key k, Context& ctx, uint8_t opcode, Source src);
    SimpleInstruction(
      Key k, Context& ctx, uint8_t opcode, TupleTypeMapT<Args>... args)
      : InstructionBase{k, ctx, opcode}, args{std::move(args)...} {}

    static const FilePosition SIZE;
    FilePosition GetSize() const noexcept override { return SIZE; }

    using ArgsT = std::tuple<TupleTypeMapT<Args>...>;
    ArgsT args;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void PostInsert(Flavor f) override;
  };

  class InstructionRndJumpItem final : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    InstructionRndJumpItem(Key k, Context& ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override { return 2 + tgts.size()*4; }

    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    std::vector<Libshit::NotNull<LabelPtr>> tgts;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void PostInsert(Flavor f) override;
  };

  class UnimplementedInstructionItem final : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    UnimplementedInstructionItem(
      Key k, Context& ctx, uint8_t opcode, const Source&)
      : InstructionBase{k, ctx, opcode}
    { LIBSHIT_THROW(Libshit::DecodeError, "Unimplemented instruction"); }

    FilePosition GetSize() const noexcept override { return 0; }

  private:
    void Dump_(Sink&) const override {}
    void Inspect_(std::ostream&, unsigned) const override {}
    void PostInsert(Flavor) override {}
  };

  class InstructionJumpIfItem final : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct FixParams
    {
      boost::endian::little_uint16_t size;
      boost::endian::little_uint32_t tgt;

      void Validate(FilePosition rem_size, FilePosition size);
    };
    static_assert(sizeof(FixParams) == 6);

    struct NodeParams
    {
      boost::endian::little_uint8_t operation;
      boost::endian::little_uint32_t value;
      boost::endian::little_uint16_t left;
      boost::endian::little_uint16_t right;

      void Validate(uint16_t size);
    };
    static_assert(sizeof(NodeParams) == 9);

    struct Node : Libshit::Lua::ValueObject
    {
      uint8_t operation = 0;
      uint32_t value = 0;
      size_t left = 0, right = 0;

      Node() = default;
      Node(uint8_t operation, uint32_t value, size_t left, size_t right)
        : operation{operation}, value{value}, left{left}, right{right} {}
      LIBSHIT_LUA_CLASS;
    };

    InstructionJumpIfItem(Key k, Context& ctx, uint8_t opcode, Source src);
    InstructionJumpIfItem(Key k, Context& ctx, uint8_t opcode,
                          Libshit::NotNull<LabelPtr> tgt, std::vector<Node> tree)
      : InstructionBase{k, ctx, opcode}, tgt{tgt}, tree{Libshit::Move(tree)} {}
#if LIBSHIT_WITH_LUA
    InstructionJumpIfItem(
      Key k, Context& ctx, Libshit::Lua::StateRef vm, uint8_t opcode,
      Libshit::NotNull<LabelPtr> tgt, Libshit::Lua::RawTable tree);
#endif

    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + tree.size() * sizeof(NodeParams); }

    Libshit::NotNull<LabelPtr> tgt;
    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    std::vector<Node> tree;

    void Dispose() noexcept override;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void InspectNode(std::ostream& os, size_t i) const;
    void PostInsert(Flavor f) override;
  };

  class InstructionJumpSwitchItemNoire : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct FixParams
    {
      boost::endian::little_uint32_t expected_val;
      boost::endian::little_uint16_t size;

      void Validate(FilePosition rem_size);
    };
    static_assert(sizeof(FixParams) == 6);

    struct ExpressionParams
    {
      boost::endian::little_uint32_t expression;
      boost::endian::little_uint32_t tgt;

      void Validate(FilePosition size);
    };
    static_assert(sizeof(ExpressionParams) == 8);

    struct Expression : Libshit::Lua::ValueObject
    {
      uint32_t expression;
      Libshit::NotNull<LabelPtr> target;

      Expression(uint32_t expression, Libshit::NotNull<LabelPtr> target)
        : expression{expression}, target{std::move(target)} {}
      LIBSHIT_LUA_CLASS;
    };


    InstructionJumpSwitchItemNoire(
      Key k, Context& ctx, uint8_t opcode, Source src);
    InstructionJumpSwitchItemNoire(
      Key k, Context& ctx, uint8_t opcode, uint32_t expected_val,
      bool last_is_default, Libshit::AT<std::vector<Expression>> expressions)
      : InstructionBase{k, ctx, opcode}, expected_val{expected_val},
        last_is_default{last_is_default},
        expressions{Libshit::Move(expressions)} {}


    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + expressions.size() * sizeof(ExpressionParams); }

    uint32_t expected_val;
    bool last_is_default;

    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    std::vector<Expression> expressions;

    void Dispose() noexcept override;

  protected:
    InstructionJumpSwitchItemNoire(Key k, Context& ctx, uint8_t opcode)
      : InstructionBase{k, ctx, opcode} {}

    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void InspectBase(std::ostream& os, unsigned indent) const;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void PostInsert(Flavor f) override;
  };

  class InstructionJumpSwitchItemPotbb final : public InstructionJumpSwitchItemNoire
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:

    InstructionJumpSwitchItemPotbb(
      Key k, Context& ctx, uint8_t opcode, Source src);
    InstructionJumpSwitchItemPotbb(
      Key k, Context& ctx, uint8_t opcode, uint32_t expected_val,
      bool last_is_default, Libshit::AT<std::vector<Expression>> expressions,
      uint8_t trailing_byte)
      : InstructionJumpSwitchItemNoire{
      k, ctx, opcode, expected_val, last_is_default, Libshit::Move(expressions)},
        trailing_byte{trailing_byte} {}

    FilePosition GetSize() const noexcept override
    { return 1 + InstructionJumpSwitchItemNoire::GetSize(); }

    uint8_t trailing_byte;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
  };

  // compile time map of opcode->instruction classes
  template <Flavor F, uint8_t Opcode> struct InstructionMap
  { using Type = SimpleInstruction<false>; };

  // In POTBB dummy instructions skip one byte...
  template <uint8_t Opcode> struct InstructionMap<Flavor::POTBB, Opcode>
  { using Type = SimpleInstruction<false, uint8_t>; };

#define NEPTOOLS_OPCODE(flavor, id, ...) \
  template<> struct InstructionMap<Flavor::flavor, id> \
  { using Type = __VA_ARGS__; }
#define NEPTOOLS_SIMPLE_OPCODE(flavor, id, ...)        \
  NEPTOOLS_OPCODE(flavor, id, SimpleInstruction<__VA_ARGS__>)

  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x01, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x04, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x05, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x06, false, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x07, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0a, false, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0b, false, uint8_t, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0c, false, uint8_t);
  NEPTOOLS_OPCODE(NOIRE, 0x0d, InstructionRndJumpItem);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0e, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0f, true,  std::string, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x10, false, Tagged, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x11, false, Tagged, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x12, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x13, false, Tagged, uint8_t, uint32_t, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x14, false, Tagged, uint8_t, uint32_t, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x15, false, Tagged, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x16, false, Tagged, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x17, false, Tagged, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x18, false, Tagged, Tagged);
  NEPTOOLS_OPCODE(NOIRE, 0x19, UnimplementedInstructionItem); // noreturn
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x1a, false, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x1b, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x1c, false, uint8_t);
  NEPTOOLS_OPCODE(NOIRE, 0x1d, InstructionJumpIfItem);
  NEPTOOLS_OPCODE(NOIRE, 0x1e, InstructionJumpSwitchItemNoire);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x1f, false, Tagged, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x20, false, uint8_t, void*, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x21, false, void*, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x22, false, void*, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x23, false, void*, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x24, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x30, false, std::string, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x31, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x32, false, float, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x33, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x34, false, uint32_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x35, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x36, false, uint32_t, float, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x37, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x38, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x39, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x3a, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x3f, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x40, false, Tagged, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x41, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x42, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x43, false, uint16_t, uint16_t, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x44, false, float, uint16_t, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x45, false, uint8_t, uint32_t, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x46, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x47, false, uint8_t, uint16_t, uint16_t, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x48, false, uint8_t, uint16_t, uint16_t, uint16_t,  uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x49, false, uint8_t, float, uint16_t, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4a, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4b, false, uint8_t, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4c, false, uint8_t, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4d, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4e, false, uint8_t, float, float, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x4f, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x50, false, uint16_t, uint16_t, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x51, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x53, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x54, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x55, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x57, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x58, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x59, false, uint8_t, uint8_t, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5a, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5b, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5c, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5d, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5e, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x5f, false, uint16_t, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x60, false, Tagged, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x61, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x62, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x63, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x64, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x65, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x66, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x67, false, uint32_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x68, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x69, false, uint8_t, uint32_t, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6a, false, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6b, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6c, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6d, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6e, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x6f, false, std::string, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x70, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x71, true, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x73, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x74, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x75, false, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x76, false, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x77, false, uint8_t, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x78, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x79, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7a, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7b, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7c, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7d, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7e, false, uint8_t, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x7f, false, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x80, false, uint32_t, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x81, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x82, false, uint32_t, uint32_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x83, false, uint32_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x84, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x87, false, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x88, false, uint16_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x89, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8a, false, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8b, false, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8c, false, uint32_t, uint32_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8d, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8e, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x8f, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x90, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x91, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x92, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x93, false, std::string, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x94, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x95, false, std::string, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x96, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x97, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x98, false, uint16_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x99, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa0, false, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa1, false, uint8_t, void*, uint16_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa2, false, uint16_t, void*, uint16_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa3, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa5, false, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa6, false, uint8_t, void*, uint16_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa7, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa8, false, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xa9, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xaa, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xab, false, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xb0, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xb1, false, uint16_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xb3, false, uint16_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xb4, false, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xb5, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd0, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd1, false, uint32_t, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd2, false, uint32_t, uint8_t, uint8_t, float);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd3, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd4, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd5, false, uint32_t, uint16_t, float, float, float, float, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd6, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd7, false, uint32_t, float, float, float, float, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd8, false, uint8_t, uint8_t, float, float, float, float, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xd9, false, uint32_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xda, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xdb, false, uint8_t, uint16_t, uint8_t, uint8_t, float, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xdc, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xdd, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xdf, false, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf0, false, uint8_t, Tagged);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf1, false, uint8_t, uint16_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf2, false, uint8_t, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf3, false, uint8_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf4, false, std::string);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf5, false, uint16_t, uint8_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0xf7, false, uint16_t, uint8_t);

  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x00, false); // Nop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x01, true);  // Exit
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x02, true);  // Cont
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x03, false, uint8_t /*ignored*/); // Endv
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x04, true);  // InfiniWait (infinite loop)
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x05, false, uint32_t); // VWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x06, false, Code*); // Goto (actually, call)
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x07, true);  // Return
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x08, false); // RetAddrPush
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x09, false); // RetAddrPop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x0a, false, uint32_t /*ignored*/); // Call1V
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x0b, false, uint8_t, void*); // SubStart
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x0c, false, uint8_t); // SubStop
  NEPTOOLS_OPCODE(POTBB, 0x0d, InstructionRndJumpItem); // RndJump
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x0e, false, std::string /*ignored*/); // Printf
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x0f, true,  std::string); // FileJump
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x10, false, Tagged, Code*); // IsFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x11, false, Tagged, Code*); // NgFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x12, false, Tagged, uint8_t); // FlgSw
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x13, false, Tagged, uint8_t, Tagged, Code*); // IsPrm
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x14, false, Tagged, uint8_t, Tagged, Code*); // NgPrm
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x15, false, Tagged, Tagged); // SetPrm
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x16, false, Tagged, Tagged); // SetPrmWk
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x17, false, Tagged, Tagged); // AddPrm
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x18, false, Tagged, Tagged); // AddPrmWk
  // Tagged, Code*... (unknown count, impossible to determine)
  NEPTOOLS_OPCODE(POTBB, 0x19, UnimplementedInstructionItem); // noreturn, PrmBranch
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x1a, false, Code*); // Call
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x1b, true); // CallRet
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x1c, true, uint8_t); // SubEndWait
  NEPTOOLS_OPCODE(POTBB, 0x1d, InstructionJumpIfItem); // JumpIf
  NEPTOOLS_OPCODE(POTBB, 0x1e, InstructionJumpSwitchItemPotbb); // JumpSwitch
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x1f, false, Tagged, uint16_t); // RandPrm
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x20, false, uint8_t, void*, void*); // DataBaseParam
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x21, false, uint16_t); // CourseFlag
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x22, false, uint16_t); // SetNowScenario
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x23, false, uint16_t); // CharEntry
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x24, false, uint16_t); // CrossFade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x25, false, uint8_t, uint16_t, uint8_t); // PatternCrossFade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x26, false, uint8_t); // DispTone
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x27, false, uint8_t); // Monologue
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x28, false, uint16_t, uint8_t /*unused*/, uint8_t, uint16_t, uint16_t, uint16_t, float, uint8_t); // ExiPlay
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x29, false, uint8_t /*unused*/, uint8_t); // ExiStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2a, false, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t); // PatternFade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2b, false, float, uint8_t, uint8_t); // Ambiguous
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2c, false, float, float, uint16_t); // AmbiguousFade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2d, false, uint32_t /*unused*/); // TouchWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2e, false, Tagged, uint16_t); // CourseFlagGet
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x2f, false, uint16_t); // Chapter
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x30, false, std::string, uint8_t, uint8_t); // Movie
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x31, false, uint8_t, uint16_t); // BgmPlay
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x32, false, float, uint16_t); // BgmVolume
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x33, false, uint32_t, uint8_t); // SePlay
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x34, false, uint32_t, uint16_t); // SeStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x35, false, uint32_t, uint8_t); // SeWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x36, false, uint32_t, float, uint16_t); // SeVolume
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x37, false, uint16_t); // SeAllStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x38, false, std::string); // VoicePlay
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x39, false, uint16_t); // VoiceStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x3a, false, uint8_t); // VoiceWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x3b, false, Tagged); // VoiceVolumePlay
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x3e, false, uint32_t /*unused*/); // BackLogReset
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x3f, false, Tagged); // GetCountry
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x40, false, uint32_t, uint16_t, uint8_t); // BgOpen
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x41, false, uint16_t); // BgClose
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x42, false, uint16_t, uint16_t); // BgFrame
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x43, false, uint16_t, uint16_t, uint16_t, uint8_t); // BgMove
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x44, false, float, uint16_t, uint8_t, uint8_t); // BgScale
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x45, false, uint8_t, uint32_t, uint8_t, uint16_t); // BustOpen
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x46, false, uint8_t, uint16_t); // BustClose
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x47, false, uint8_t, uint16_t, uint16_t, uint16_t, uint8_t); // BustClose
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x48, false, uint8_t, uint16_t, uint16_t, uint16_t /*unused*/, uint8_t); // BustMoveAdd
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x49, false, uint8_t, float, uint16_t, uint8_t, uint8_t); // BustScale
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4a, false, uint8_t, uint8_t); // BustPriority
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4b, false, uint8_t, uint8_t, uint8_t, uint16_t); // BustQuake
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4c, false, uint8_t); // SetEntryCharFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4d, false, uint16_t /*unused*/); // BustTone
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4e, false, uint8_t, float, float, uint16_t); // BustFade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x4f, false, std::string); // Name
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x50, false, uint16_t, uint16_t, std::string, uint8_t, uint16_t); // Message
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x51, false, uint8_t); // MessageWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x52, false); // MessageWinClose
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x53, false, uint8_t); // MessageFontSize
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x54, false, uint16_t, uint16_t); // MessageQuake
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x55, false, uint8_t); // Trophy
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x56, false); // MessageDelete
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x57, false, uint16_t, uint16_t); // Quake
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x58, false, uint8_t, uint8_t, uint16_t); // Fade
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x59, false, Code* /*guess*/, std::string, uint16_t); // Choice
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5a, true);  // ChoiceStart
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5b, false, Tagged); // GetBg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5c, false, uint16_t); // FontColor
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5d, false, uint8_t); // WorldType
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5e, false, Tagged); // GetWorld
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x5f, true, uint16_t); // FlowChart
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x60, true, std::string); // MiniGame
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x61, false, Tagged, uint16_t); // CourseOpenGet
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x62, false); // SystemSave
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x63, false, uint8_t); // BackLogNg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x64, false, uint8_t); // EventSkipNg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x65, false, uint8_t); // EventAutoNg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x66, false, uint8_t); // StopNg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x67, false); // DataSave
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x68, false); // SkipStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x69, false, std::string, std::string); // MessageVoice
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x6a, false, std::string, std::string); // MessageVoice2
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x6b, false, uint8_t); // SaveNg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x6f, false, std::string, uint8_t); // Dialog
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x70, false, uint16_t, uint8_t); // ExiLoopStop
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x71, false, uint16_t, uint8_t); // ExiEndWait
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x72, false, uint8_t); // ClearSet
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x73, false, std::string); // SaveTitleSet
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x78, false, uint16_t); // VideoFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x79, false, uint16_t, uint16_t /*unused*/); // AlbumFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x7a, false, uint8_t); // WaitFlg
  NEPTOOLS_SIMPLE_OPCODE(POTBB, 0x7b, false, uint8_t); // AudioFlg


#undef NEPTOOLS_OPCODE
#undef NEPTOOLS_SIMPLE_OPCODE

  template <Flavor F, uint8_t Opcode>
  using InstructionItem = typename InstructionMap<F, Opcode>::Type;
}

#endif
