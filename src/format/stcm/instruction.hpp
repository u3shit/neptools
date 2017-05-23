#ifndef UUID_032F4E0D_CAAE_429A_9928_2ACC24EC51E1
#define UUID_032F4E0D_CAAE_429A_9928_2ACC24EC51E1
#pragma once

#include "../item.hpp"
#include "../../check.hpp"
#include "../../source.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
class RawItem;

namespace Stcm
{

class InstructionItem final : public ItemWithChildren
{
    NEPTOOLS_DYNAMIC_OBJECT;
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
    NEPTOOLS_STATIC_ASSERT(sizeof(Header) == 0x10);

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
        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Parameter) == 0xc);

    InstructionItem(Key k, Context* ctx) : ItemWithChildren{k, ctx} {}
    InstructionItem(Key k, Context* ctx, Source src);
    static InstructionItem& CreateAndInsert(ItemPointer ptr);

    FilePosition GetSize() const noexcept override;
    void Fixup() override;
    void Dispose() noexcept override;

    bool IsCall() const noexcept { return is_call; }
    template <typename Checker = Check::Assert>
    uint32_t GetOpcode() const noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(std::logic_error, !is_call, "Instruction is call");
        return opcode;
    }
    void SetOpcode(uint32_t oc) noexcept { is_call = false; opcode = oc; }

    template <typename Checker = Check::Assert>
    const Label* GetTarget() const noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(std::logic_error, is_call, "Instruction is not call");
        return target;
    }
    void SetTarget(const Label* label) noexcept
    { is_call = false; target = label; }


    class Param48
    {
    public:
        enum class Type
        {
            MEM_OFFSET,
            IMMEDIATE,
            INDIRECT,
            READ_STACK,
            READ_4AC,
        };

        Param48() : type{Type::IMMEDIATE}, num{0} {}
        Param48(Context& ctx, uint32_t val);
        uint32_t Dump() const noexcept;

        Type GetType() const noexcept { return type; }

#define NEPTOOLS_GEN_GETSET(type, Camel, check, check_set, ret, set)        \
        template <typename Checker = Check::Assert>                         \
        type Get##Camel() const noexcept(Checker::IS_NOEXCEPT)              \
        {                                                                   \
            NEPTOOLS_CHECK(std::logic_error, check, "Invalid type");        \
            return ret;                                                     \
        }                                                                   \
        template <typename Checker = Check::Assert>                         \
        void Set##Camel(type nval) noexcept                                 \
        {                                                                   \
            NEPTOOLS_CHECK(std::logic_error, check, "Invalid type");        \
            NEPTOOLS_CHECK(std::logic_error, check_set, "Invalid value");   \
            set;                                                            \
        }
        NEPTOOLS_GEN_GETSET(
            const Label&, Label, type == Type::MEM_OFFSET, true, *label, label = &nval)
        NEPTOOLS_GEN_GETSET(
            uint32_t, Value, type != Type::MEM_OFFSET,
            (type == Type::IMMEDIATE || type == Type::INDIRECT) ?
              Parameter::TypeTag(nval) == 0 : nval < 16,
            num, num = nval)


#define NEPTOOLS_GEN_LABEL(Camel, SNAKE, cls)                                   \
        template <typename Checker = Check::Assert>                             \
        const Label& Get##Camel() const noexcept(Checker::IS_NOEXCEPT)          \
        {                                                                       \
            NEPTOOLS_CHECK(                                                     \
                std::logic_error, type == Type::SNAKE, "Invalid type");         \
            return *label;                                                      \
        }                                                                       \
        void Set##Camel(const Label& nlabel) noexcept                           \
        { type = Type::SNAKE; label = &nlabel; }                                \
        static cls Camel(const Label& nlabel) noexcept                          \
        { cls p; p.Set##Camel(nlabel); return p; }
        NEPTOOLS_GEN_LABEL(MemOffset, MEM_OFFSET, Param48)

#define NEPTOOLS_GEN_INT(Camel, SNAKE, cls, check)                              \
        template <typename Checker = Check::Assert>                             \
        uint32_t Get##Camel() const noexcept(Checker::IS_NOEXCEPT)              \
        {                                                                       \
            NEPTOOLS_CHECK(                                                     \
                std::logic_error, type == Type::SNAKE, "Invalid type");         \
            return num;                                                         \
        }                                                                       \
        template <typename Checker = Check::Assert>                             \
        void Set##Camel(uint32_t val) noexcept(Checker::IS_NOEXCEPT)            \
        {                                                                       \
            NEPTOOLS_CHECK(std::logic_error, check, "Invalid value");           \
            type = Type::SNAKE;                                                 \
            num = val;                                                          \
        }                                                                       \
        template <typename Checker = Check::Assert>                             \
        static cls Camel(uint32_t val) noexcept(Checker::IS_NOEXCEPT)           \
        { cls p; p.Set##Camel<Checker>(val); return p; }

        NEPTOOLS_GEN_INT(Immediate, IMMEDIATE,  Param48, Parameter::TypeTag(val) == 0)
        NEPTOOLS_GEN_INT(Indirect,  INDIRECT,   Param48, Parameter::TypeTag(val) == 0)
        NEPTOOLS_GEN_INT(ReadStack, READ_STACK, Param48, val < 16)
        NEPTOOLS_GEN_INT(Read4ac,   READ_4AC,   Param48, val < 16)

    private:
        Type type;
        union
        {
            const Label* label;
            uint32_t num;
        };
    };

    class Param
    {
    public:
        enum class Type
        {
            MEM_OFFSET,
            INSTR_PTR0,
            INSTR_PTR1,
            COLL_LINK,

            INDIRECT,
            READ_STACK,
            READ_4AC,
        };

        Param(Context& ctx, const Parameter& p);

        void Dump(Sink& sink) const;
        void Dump(Sink&& sink) const { Dump(sink); }

        Type GetType() const noexcept { return type; }

        NEPTOOLS_GEN_GETSET(
            const Label&, Label, type <= Type::COLL_LINK, true, *label, label = &nval)
        NEPTOOLS_GEN_GETSET(
            uint32_t, Value, type >= Type::INDIRECT,
            type == Type::INDIRECT ? Parameter::TypeTag(nval) == 0 : nval < 16,
            num, num = nval)
#undef NEPTOOLS_GEN_GETSET

        NEPTOOLS_GEN_LABEL(MemOffset, MEM_OFFSET, Param)
        NEPTOOLS_GEN_INT  (Indirect,  INDIRECT,   Param, Parameter::TypeTag(val) == 0)
        NEPTOOLS_GEN_INT  (ReadStack, READ_STACK, Param, val < 16)
        NEPTOOLS_GEN_INT  (Read4ac,   READ_4AC,   Param, val < 16)
        NEPTOOLS_GEN_LABEL(InstrPtr0, INSTR_PTR0, Param)
        NEPTOOLS_GEN_LABEL(InstrPtr1, INSTR_PTR1, Param)
        NEPTOOLS_GEN_LABEL(CollLink,  COLL_LINK,  Param)
#undef NEPTOOLS_GEN_LABEL
#undef NEPTOOLS_GEN_INT

        bool HasParam4() const noexcept { return type == Type::MEM_OFFSET; }
        bool HasParam8() const noexcept
        { return type == Type::MEM_OFFSET || type == Type::INDIRECT; }

        template <typename Checker = Check::Assert>
        const Param48& GetParam4() const noexcept(Checker::IS_NOEXCEPT)
        {
            NEPTOOLS_CHECK(std::logic_error, HasParam4(), "Invalid type");
            return param_4;
        }
        template <typename Checker = Check::Assert>
        Param48& GetParam4() noexcept
        {
            NEPTOOLS_CHECK(std::logic_error, HasParam4(), "Invalid type");
            return param_4;
        }

        template <typename Checker = Check::Assert>
        const Param48& GetParam8() const noexcept
        {
            NEPTOOLS_CHECK(std::logic_error, HasParam8(), "Invalid type");
            return param_8;
        }
        template <typename Checker = Check::Assert>
        Param48& GetParam8() noexcept
        {
            NEPTOOLS_CHECK(std::logic_error, HasParam8(), "Invalid type");
            return param_8;
        }

    private:
        Param() = default;

        Type type;
        union
        {
            const Label* label;
            uint32_t num;
        };
        Param48 param_4, param_8;
    };
    std::vector<Param> params;

private:
    bool is_call;
    union
    {
        uint32_t opcode;
        const Label* target;
    };

    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void Parse_(Source& src);
};

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p);
std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p);

}
}
#endif
