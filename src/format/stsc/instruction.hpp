#ifndef UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#define UUID_0A01B0B9_DA9A_4A05_919B_A5503594CA9D
#pragma once

#include "../../source.hpp"
#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stsc
{

class InstructionBase : public Item
{
    NEPTOOLS_LUA_CLASS;
public:
    InstructionBase(Key k, Context& ctx, uint8_t opcode)
        : Item{k, ctx}, opcode{opcode} {}

    static InstructionBase& CreateAndInsert(ItemPointer ptr);

    const uint8_t opcode;

protected:
    void InstrDump(Sink& sink) const;
    std::ostream& InstrInspect(std::ostream& os) const;

private:
    virtual void PostInsert() = 0;
};

using Tagged = uint32_t;
struct Code;

template <typename T> struct TupleTypeMap { using Type = T; };
template<> struct TupleTypeMap<std::string> { using Type = LabelPtr; };
template<> struct TupleTypeMap<void*> { using Type = LabelPtr; };
template<> struct TupleTypeMap<Code*> { using Type = LabelPtr; };

template <bool NoReturn, typename... Args>
class SimpleInstruction final : public InstructionBase
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    SimpleInstruction(Key k, Context& ctx, uint8_t opcode, Source src);

    static const FilePosition SIZE;
    FilePosition GetSize() const noexcept override { return SIZE; }

    std::tuple<typename TupleTypeMap<Args>::Type...> args;

private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void PostInsert() override;
};

class Instruction0dItem final : public InstructionBase
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    Instruction0dItem(Key k, Context& ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override { return 2 + tgts.size()*4; }

    NEPTOOLS_LUAGEN(get="::Neptools::Lua::GetSmartOwnedMember")
    std::vector<NotNull<LabelPtr>> tgts;

private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void PostInsert() override;
};

class UnimplementedInstructionItem final : public InstructionBase
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    UnimplementedInstructionItem(
        Key k, Context& ctx, uint8_t opcode, const Source&)
        : InstructionBase{k, ctx, opcode}
    { NEPTOOLS_THROW(DecodeError{"Unimplemented instruction"}); }

    FilePosition GetSize() const noexcept override { return 0; }

private:
    void Dump_(Sink&) const override {}
    void Inspect_(std::ostream&) const override {}
    void PostInsert() override {}
};

class Instruction1dItem final : public InstructionBase
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    struct FixParams
    {
        boost::endian::little_uint16_t size;
        boost::endian::little_uint32_t tgt;

        void Validate(FilePosition rem_size, FilePosition size);
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(FixParams) == 6);

    struct NodeParams
    {
        boost::endian::little_uint8_t operation;
        boost::endian::little_uint32_t value;
        boost::endian::little_uint16_t left;
        boost::endian::little_uint16_t right;

        void Validate(uint16_t size);
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(NodeParams) == 9);

    Instruction1dItem(Key k, Context& ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + tree.size() * sizeof(NodeParams); }

    NotNull<LabelPtr> tgt;
    struct Node : Lua::ValueObject
    {
        uint8_t operation;
        uint32_t value;
        size_t left, right;

        Node(uint8_t operation, uint32_t value, size_t left, size_t right)
            : operation{operation}, value{value}, left{left}, right{right} {}
        NEPTOOLS_LUA_CLASS;
    };
    NEPTOOLS_LUAGEN(get="::Neptools::Lua::GetSmartOwnedMember")
    std::vector<Node> tree;

    void Dispose() noexcept override;

private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void InspectNode(std::ostream& os, size_t i) const;
    void PostInsert() override;
};

class Instruction1eItem final : public InstructionBase
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    struct FixParams
    {
        boost::endian::little_uint32_t field_0;
        boost::endian::little_uint16_t size;

        void Validate(FilePosition rem_size);
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(FixParams) == 6);

    struct ExpressionParams
    {
        boost::endian::little_uint32_t expression;
        boost::endian::little_uint32_t tgt;

        void Validate(FilePosition size);
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(ExpressionParams) == 8);

    Instruction1eItem(Key k, Context& ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + expressions.size() * sizeof(ExpressionParams); }

    uint32_t field_0;
    bool flag;

    struct Expression : Lua::ValueObject
    {
        uint32_t expression;
        NotNull<LabelPtr> target;

        Expression(uint32_t expression, NotNull<LabelPtr> target)
            : expression{expression}, target{std::move(target)} {}
        NEPTOOLS_LUA_CLASS;
    };
    NEPTOOLS_LUAGEN(get="::Neptools::Lua::GetSmartOwnedMember")
    std::vector<Expression> expressions;

    void Dispose() noexcept override;

private:
    void Parse_(Context& ctx, Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void PostInsert() override;
};

// compile time map of opcode->instruction classes
template <uint8_t Opcode> struct InstructionMap
{ using Type = SimpleInstruction<false>; };

#define NEPTOOLS_OPCODE(id, ...)                \
    template<> struct InstructionMap<id>        \
    { using Type = __VA_ARGS__; }
#define NEPTOOLS_SIMPLE_OPCODE(id, ...)                 \
    NEPTOOLS_OPCODE(id, SimpleInstruction<__VA_ARGS__>)

NEPTOOLS_SIMPLE_OPCODE(0x01, true);
NEPTOOLS_SIMPLE_OPCODE(0x05, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x06, false, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x07, true);
NEPTOOLS_SIMPLE_OPCODE(0x0a, false, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x0b, false, uint8_t, void*);
NEPTOOLS_SIMPLE_OPCODE(0x0c, false, uint8_t);
NEPTOOLS_OPCODE(0x0d, Instruction0dItem); // noreturn
NEPTOOLS_SIMPLE_OPCODE(0x0e, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x0f, true,  std::string, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x10, false, Tagged, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x11, false, Tagged, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x12, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x13, false, Tagged, uint8_t, uint32_t, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x14, false, Tagged, uint8_t, uint32_t, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x15, false, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x16, false, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x17, false, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x18, false, Tagged, Tagged);
NEPTOOLS_OPCODE(0x19, UnimplementedInstructionItem); // noreturn
NEPTOOLS_SIMPLE_OPCODE(0x1a, false, Code*);
NEPTOOLS_SIMPLE_OPCODE(0x1b, true);
NEPTOOLS_SIMPLE_OPCODE(0x1c, false, uint8_t);
NEPTOOLS_OPCODE(0x1d, Instruction1dItem);
NEPTOOLS_OPCODE(0x1e, Instruction1eItem);
NEPTOOLS_SIMPLE_OPCODE(0x1f, false, Tagged, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x20, false, uint8_t, void*, void*);
NEPTOOLS_SIMPLE_OPCODE(0x21, false, void*, void*);
NEPTOOLS_SIMPLE_OPCODE(0x22, false, void*, void*);
NEPTOOLS_SIMPLE_OPCODE(0x23, false, void*, void*);
NEPTOOLS_SIMPLE_OPCODE(0x24, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x30, false, std::string, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x31, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x32, false, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x33, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x34, false, uint32_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x35, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x36, false, uint32_t, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x37, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x38, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x39, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x3a, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x3f, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x40, false, Tagged, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x41, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x42, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x43, false, uint16_t, uint16_t, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x44, false, float, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x45, false, uint8_t, uint32_t, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x46, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x47, false, uint8_t, uint16_t, uint16_t, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x48, false, uint8_t, uint16_t, uint16_t, uint16_t,  uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x49, false, uint8_t, float, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4a, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4b, false, uint8_t, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x4c, false, uint8_t, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x4d, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4e, false, uint8_t, float, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x4f, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x50, false, uint16_t, uint16_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x51, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x53, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x54, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x55, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x57, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x58, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x59, false, uint8_t, uint8_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5a, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x5b, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5c, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5d, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x5e, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5f, false, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x60, false, Tagged, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x61, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x62, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x63, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x64, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x65, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x66, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x67, false, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x68, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x69, false, uint8_t, uint32_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x6a, false, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6b, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6c, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x6d, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6e, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x6f, false, std::string, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x70, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x71, true, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x73, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x74, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x75, false, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x76, false, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x77, false, uint8_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x78, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x79, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x7a, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x7b, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x7c, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x7d, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x7e, false, uint8_t, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x7f, false, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x80, false, uint32_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x81, true);
NEPTOOLS_SIMPLE_OPCODE(0x82, false, uint32_t, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x83, false, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x84, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x87, false, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x88, false, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x89, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x8a, false, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8b, false, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8c, false, uint32_t, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8d, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x8e, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8f, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x90, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x91, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x92, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x93, false, std::string, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x94, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x95, false, std::string, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x96, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x97, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x98, false, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x99, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xa0, false, void*);
NEPTOOLS_SIMPLE_OPCODE(0xa1, false, uint8_t, void*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa2, false, uint16_t, void*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa3, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xa5, false, void*);
NEPTOOLS_SIMPLE_OPCODE(0xa6, false, uint8_t, void*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa7, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xa8, false, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xa9, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xaa, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xab, false, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb0, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xb1, false, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb3, false, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb4, false, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xb5, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd0, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd1, false, uint32_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd2, false, uint32_t, uint8_t, uint8_t, float);
NEPTOOLS_SIMPLE_OPCODE(0xd3, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd4, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd5, false, uint32_t, uint16_t, float, float, float, float, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd6, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0xd7, false, uint32_t, float, float, float, float, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd8, false, uint8_t, uint8_t, float, float, float, float, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd9, false, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xda, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdb, false, uint8_t, uint16_t, uint8_t, uint8_t, float, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdc, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdd, false, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0xdf, false, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf0, false, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xf1, false, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xf2, false, uint8_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xf3, false, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf4, false, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xf5, false, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf7, false, uint16_t, uint8_t);

#undef NEPTOOLS_OPCODE
#undef NEPTOOLS_SIMPLE_OPCODE

template <uint8_t Opcode>
using InstructionItem = typename InstructionMap<Opcode>::Type;
}
}

#endif
