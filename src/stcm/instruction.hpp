#ifndef STCM_INSTRUCTION_HPP
#define STCM_INSTRUCTION_HPP
#pragma once

#include "../item.hpp"
#include <boost/endian/arithmetic.hpp>

class RawItem;

namespace Stcm
{

struct Instruction
{
    boost::endian::little_uint32_t is_call;
    boost::endian::little_uint32_t opcode;
    boost::endian::little_uint32_t param_count;
    boost::endian::little_uint32_t size;

    enum Opcode
    {
        USER_OPCODES = 0x1000,
        SYSTEM_OPCODES_BEGIN = 0xffffff00,
        SYSTEM_OPCODES_END   = 0xffffff14,
    };

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
        bool IsValid(size_t file_size) const noexcept;
    } params[1];

    bool IsValid(size_t file_size) const noexcept;
    static constexpr size_t SIZE = 0x10;
};
static_assert(sizeof(Instruction::Parameter) == 0xc, "");
static_assert(sizeof(Instruction) - sizeof(Instruction::Parameter) == 0x10, "");

class InstructionItem final : public ItemWithChildren
{
public:
    InstructionItem(Key k, Context* ctx) : ItemWithChildren{k, ctx} {}
    InstructionItem(Key k, Context* ctx, const Instruction* instr);
    static void MaybeCreate(ItemPointer ptr);
    static InstructionItem* CreateAndInsert(ItemPointer ptr);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override;
    size_t UpdatePositions(FilePosition npos) override;

    bool is_call;
    union
    {
        uint32_t opcode;
        const Label* target;
    };

    struct Param48
    {
        enum Type
        {
            MEM_OFFSET,
            IMMEDIATE,
            INDIRECT,
            READ_STACK,
            READ_4AC,
        } type;
        union
        {
            const Label* label;
            uint32_t num;
        };
    };
    struct Param
    {
        enum Type
        {
            MEM_OFFSET,
            INDIRECT,
            READ_STACK,
            READ_4AC,
            INSTR_PTR0,
            INSTR_PTR1,
            COLL_LINK,
        } type;
        union
        {
            const Label* label;
            uint32_t num;
        } param_0;
        Param48 param_4, param_8;
    };
    std::vector<Param> params;

private:
    void Dump48(boost::endian::little_uint32_t& out, const Param48& in) const noexcept;
    void ConvertParam(Param& out, const Instruction::Parameter& in);
    void ConvertParam48(Param48& out, uint32_t in);
};

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p);
std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p);

}

#endif
