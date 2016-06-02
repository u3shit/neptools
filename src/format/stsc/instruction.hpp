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
public:
    InstructionBase(Key k, Context* ctx, uint8_t opcode)
        : Item{k, ctx}, opcode{opcode} {}

    static InstructionBase* CreateAndInsert(ItemPointer ptr);

    uint8_t opcode;

protected:
    void InstrDump(Sink& sink) const;
    std::ostream& InstrInspect(std::ostream& os) const;
};

using Tagged = uint32_t;

template <typename T> struct TupleTypeMap { using Type = T; };
template<> struct TupleTypeMap<std::string> { using Type = const Label*; };

template <typename... Args>
class SimpleInstruction final : public InstructionBase
{
public:
    SimpleInstruction(Key k, Context* ctx, uint8_t opcode, Source src);

    static const FilePosition SIZE;
    FilePosition GetSize() const noexcept override { return SIZE; }

    std::tuple<typename TupleTypeMap<Args>::Type...> args;

private:
    void Parse_(Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

class Instruction0dItem final : public InstructionBase
{
public:
    Instruction0dItem(Key k, Context* ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override { return 2 + tgts.size()*4; }

    std::vector<const Label*> tgts;

private:
    void Parse_(Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

class Instruction19Item final : public InstructionBase
{
public:
    Instruction19Item(Key k, Context* ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override { return 0; }

private:
    void Parse_(Source& src) {}
    void Dump_(Sink& sink) const override {}
    void Inspect_(std::ostream& os) const override {}
};

class Instruction1dItem final : public InstructionBase
{
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

    Instruction1dItem(Key k, Context* ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + tree.size() * sizeof(NodeParams); }

    const Label* tgt;
    struct Node
    {
        uint8_t operation;
        uint32_t value;
        size_t left, right;
    };
    std::vector<Node> tree;

private:
    void Parse_(Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void InspectNode(std::ostream& os, size_t i) const;
};

class Instruction1eItem final : public InstructionBase
{
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

    Instruction1eItem(Key k, Context* ctx, uint8_t opcode, Source src);
    FilePosition GetSize() const noexcept override
    { return 1 + sizeof(FixParams) + expressions.size() * sizeof(ExpressionParams); }

    uint32_t field_0;
    bool flag;

    std::vector<std::pair<uint32_t, const Label*>> expressions;

private:
    void Parse_(Source& src);
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
};

// compile time map of opcode->instruction classes
template <uint8_t Opcode> struct InstructionMap
{ using Type = SimpleInstruction<>; };

#define NEPTOOLS_OPCODE(id, ...)                \
    template<> struct InstructionMap<id>        \
    { using Type = __VA_ARGS__; }
#define NEPTOOLS_SIMPLE_OPCODE(id, ...)                 \
    NEPTOOLS_OPCODE(id, SimpleInstruction<__VA_ARGS__>)

NEPTOOLS_SIMPLE_OPCODE(0x05, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x06, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x0a, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x0b, uint8_t, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x0c, uint8_t);
NEPTOOLS_OPCODE(0x0d, Instruction0dItem);
NEPTOOLS_SIMPLE_OPCODE(0x0e, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x0f, std::string, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x10, Tagged, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x11, Tagged, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x12, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x13, Tagged, uint8_t, uint32_t, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x14, Tagged, uint8_t, uint32_t, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x15, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x16, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x17, Tagged, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x18, Tagged, Tagged);
NEPTOOLS_OPCODE(0x19, Instruction19Item);
NEPTOOLS_SIMPLE_OPCODE(0x1a, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x1c, uint8_t);
NEPTOOLS_OPCODE(0x1d, Instruction1dItem);
NEPTOOLS_OPCODE(0x1e, Instruction1eItem);
NEPTOOLS_SIMPLE_OPCODE(0x1f, Tagged, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x20, uint8_t, const Label*, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x21, const Label*, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x22, const Label*, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x23, const Label*, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0x24, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x30, std::string, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x31, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x32, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x33, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x34, uint32_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x35, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x36, uint32_t, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x37, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x38, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x39, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x3a, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x3f, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x40, Tagged, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x41, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x42, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x43, uint16_t, uint16_t, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x44, float, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x45, uint8_t, uint32_t, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x46, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x47, uint8_t, uint16_t, uint16_t, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x48, uint8_t, uint16_t, uint16_t, uint16_t,  uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x49, uint8_t, float, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4a, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4b, uint8_t, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x4c, uint8_t, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x4d, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x4e, uint8_t, float, float, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x4f, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x50, uint16_t, uint16_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x51, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x53, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x54, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x55, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x57, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x58, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x59, uint8_t, uint8_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5a, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x5b, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5c, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5d, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x5e, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x5f, uint16_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x60, Tagged, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x61, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x62, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x63, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x64, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x65, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x66, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x67, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x68, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x69, uint8_t, uint32_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x6a, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6b, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6c, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x6d, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x6e, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x6f, std::string, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x70, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x71, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x73, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x74, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x75, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x76, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x77, uint8_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x78, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0x79, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x7a, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x7b, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x7c, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x7d, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x7e, uint8_t, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0x7f, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x80, uint32_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0x82, uint32_t, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x83, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x84, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x87, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x88, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x89, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x8a, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8b, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8c, uint32_t, uint32_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8d, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x8e, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x8f, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x90, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x91, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x92, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x93, std::string, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x94, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x95, std::string, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x96, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x97, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0x98, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0x99, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xa0, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0xa1, uint8_t, const Label*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa2, uint16_t, const Label*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa3, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xa5, const Label*);
NEPTOOLS_SIMPLE_OPCODE(0xa6, uint8_t, const Label*, uint16_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xa7, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xa8, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xa9, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xaa, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xab, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb0, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xb1, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb3, uint16_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xb4, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xb5, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd0, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd1, uint32_t, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd2, uint32_t, uint8_t, uint8_t, float);
NEPTOOLS_SIMPLE_OPCODE(0xd3, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd4, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd5, uint32_t, uint16_t, float, float, float, float, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd6, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0xd7, uint32_t, float, float, float, float, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd8, uint8_t, uint8_t, float, float, float, float, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xd9, uint32_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xda, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdb, uint8_t, uint16_t, uint8_t, uint8_t, float, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdc, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xdd, uint32_t);
NEPTOOLS_SIMPLE_OPCODE(0xdf, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf0, uint8_t, Tagged);
NEPTOOLS_SIMPLE_OPCODE(0xf1, uint8_t, uint16_t);
NEPTOOLS_SIMPLE_OPCODE(0xf2, uint8_t, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xf3, uint8_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf4, std::string);
NEPTOOLS_SIMPLE_OPCODE(0xf5, uint16_t, uint8_t);
NEPTOOLS_SIMPLE_OPCODE(0xf7, uint16_t, uint8_t);

#undef NEPTOOLS_OPCODE
#undef NEPTOOLS_SIMPLE_OPCODE

template <uint8_t Opcode>
using InstructionItem = typename InstructionMap<Opcode>::Type;
}
}

#endif
