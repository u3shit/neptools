#ifndef UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#define UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#pragma once

#include "file.hpp"
#include "../../source.hpp"
#include "../item.hpp"

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

  class Instruction0dItem final : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    Instruction0dItem(Key k, Context& ctx, uint8_t opcode, Source src);
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

  class Instruction1dItem final : public InstructionBase
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

    Instruction1dItem(Key k, Context& ctx, uint8_t opcode, Source src);
    Instruction1dItem(Key k, Context& ctx, uint8_t opcode,
                      Libshit::NotNull<LabelPtr> tgt, std::vector<Node> tree)
      : InstructionBase{k, ctx, opcode}, tgt{tgt}, tree{Libshit::Move(tree)} {}
#if LIBSHIT_WITH_LUA
    Instruction1dItem(
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

  class Instruction1eItem final : public InstructionBase
  {
    LIBSHIT_DYNAMIC_OBJECT;
  public:
    struct FixParams
    {
      boost::endian::little_uint32_t field_0;
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

    Instruction1eItem(Key k, Context& ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + expressions.size() * sizeof(ExpressionParams); }

    uint32_t field_0;
    bool flag;

    struct Expression : Libshit::Lua::ValueObject
    {
      uint32_t expression;
      Libshit::NotNull<LabelPtr> target;

      Expression(uint32_t expression, Libshit::NotNull<LabelPtr> target)
        : expression{expression}, target{std::move(target)} {}
      LIBSHIT_LUA_CLASS;
    };
    LIBSHIT_LUAGEN(get="::Libshit::Lua::GetSmartOwnedMember")
    std::vector<Expression> expressions;

    void Dispose() noexcept override;

  private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os, unsigned indent) const override;
    void PostInsert(Flavor f) override;
  };

  // compile time map of opcode->instruction classes
  template <Flavor F, uint8_t Opcode> struct InstructionMap
  { using Type = SimpleInstruction<false>; };

#define NEPTOOLS_OPCODE(flavor, id, ...) \
  template<> struct InstructionMap<Flavor::flavor, id> \
  { using Type = __VA_ARGS__; }
#define NEPTOOLS_SIMPLE_OPCODE(flavor, id, ...)        \
  NEPTOOLS_OPCODE(flavor, id, SimpleInstruction<__VA_ARGS__>)

  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x01, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x05, false, uint32_t);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x06, false, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x07, true);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0a, false, Code*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0b, false, uint8_t, void*);
  NEPTOOLS_SIMPLE_OPCODE(NOIRE, 0x0c, false, uint8_t);
  NEPTOOLS_OPCODE(NOIRE, 0x0d, Instruction0dItem); // noreturn
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
  NEPTOOLS_OPCODE(NOIRE, 0x1d, Instruction1dItem);
  NEPTOOLS_OPCODE(NOIRE, 0x1e, Instruction1eItem);
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

#undef NEPTOOLS_OPCODE
#undef NEPTOOLS_SIMPLE_OPCODE

  template <Flavor F, uint8_t Opcode>
  using InstructionItem = typename InstructionMap<F, Opcode>::Type;
}

#endif
