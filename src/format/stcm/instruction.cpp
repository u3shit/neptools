#include "instruction.hpp"
#include "data.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include <boost/assert.hpp>
#include <set>
#include <iostream>

namespace Stcm
{

static bool Param48Valid(uint32_t param, FilePosition file_size) noexcept
{
    switch (Parameter::TypeTag(param))
    {
    case Parameter::Type48::MEM_OFFSET:
        return (Parameter::Value(param) + 16) < file_size;
    case Parameter::Type48::IMMEDIATE:
        return true;
    case Parameter::Type48::INDIRECT:
        return true; // ??
    default:
        return (param >= Parameter::Type48Special::READ_STACK_MIN &&
                param <= Parameter::Type48Special::READ_STACK_MAX) ||
               (param >= Parameter::Type48Special::READ_4AC_MIN &&
                param <= Parameter::Type48Special::READ_4AC_MAX);
    }
}

bool Parameter::IsValid(FilePosition file_size) const noexcept
{
    switch (TypeTag(param_0))
    {
    case Type0::MEM_OFFSET:
        return Value(param_0) < file_size && Param48Valid(param_4, file_size) &&
            Param48Valid(param_8, file_size);
    //case Type0::UNK: todo
    case Type0::INDIRECT:
        return Value(param_0) < 256 && param_4 == 0x40000000 &&
            Param48Valid(param_8, file_size);
    case Type0::SPECIAL:
        if ((param_0 >= Type0Special::READ_STACK_MIN && param_0 <= Type0Special::READ_STACK_MAX) ||
            (param_0 >= Type0Special::READ_4AC_MIN && param_0 <= Type0Special::READ_4AC_MAX))
            return param_4 == 0x40000000 && param_8 == 0x40000000;
        else if (param_0 == Type0Special::INSTR_PTR0 ||
                 param_0 == Type0Special::INSTR_PTR1)
            return param_4 < file_size && param_8 == 0x40000000;
        else if (param_0 == Type0Special::COLL_LINK)
            return param_4 + 8 < file_size && param_8 == 0;
        else
            return false;
    default:
        return false;
    }
}

bool Instruction::IsValid(FilePosition file_size) const noexcept
{
    if ((is_call != 0 && is_call != 1) ||
        param_count >= 16 ||
        size < sizeof(Instruction) + param_count*sizeof(Parameter))
        return false;

    if (is_call)
        return opcode < file_size;
    else
        return opcode < USER_OPCODES ||
            (opcode >= SYSTEM_OPCODES_BEGIN && opcode <= SYSTEM_OPCODES_END);
}

InstructionItem::InstructionItem(Key k, Context* ctx, Source src)
    : ItemWithChildren{k, ctx}
{
    auto instr = src.Read<Instruction>();
    if (!instr.IsValid(ctx->GetSize()))
        throw std::runtime_error("invalid instruction");

    is_call = instr.is_call;
    if (is_call)
        target = ctx->GetLabelTo(instr.opcode);
    else
        opcode = instr.opcode;

    params.resize(instr.param_count);
    for (size_t i = 0; i < instr.param_count; ++i)
    {
        auto p = src.Read<Parameter>();
        if (!p.IsValid(ctx->GetSize()))
            throw std::runtime_error{"invalid instruction parameter"};
        ConvertParam(params[i], p);
    }
}

void InstructionItem::ConvertParam(Param& out, const Parameter& in)
{
    switch (Parameter::TypeTag(in.param_0))
    {
    case Parameter::Type0::MEM_OFFSET:
        out.type = Param::MEM_OFFSET;
        out.param_0.label = GetContext()->GetLabelTo(
            Parameter::Value(in.param_0));
        ConvertParam48(out.param_4, in.param_4);
        ConvertParam48(out.param_8, in.param_8);
        break;

    case Parameter::Type0::INDIRECT:
        out.type = Param::INDIRECT;
        out.param_0.num = Parameter::Value(in.param_0);
        ConvertParam48(out.param_8, in.param_8);
        break;

    case Parameter::Type0::SPECIAL:
        if (in.param_0 >= Parameter::Type0Special::READ_STACK_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_STACK_MAX)
        {
            out.type = Param::READ_STACK;
            out.param_0.num = in.param_0 - Parameter::Type0Special::READ_STACK_MIN;
        }
        else if (in.param_0 >= Parameter::Type0Special::READ_4AC_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_4AC_MAX)
        {
            out.type = Param::READ_4AC;
            out.param_0.num = in.param_0 - Parameter::Type0Special::READ_4AC_MIN;
        }
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR0)
        {
            out.type = Param::INSTR_PTR0;
            out.param_4.label = GetContext()->GetLabelTo(in.param_4);
        }
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR1)
        {
            out.type = Param::INSTR_PTR1;
            out.param_4.label = GetContext()->GetLabelTo(in.param_4);
        }
        else if (in.param_0 == Parameter::Type0Special::COLL_LINK)
        {
            out.type = Param::COLL_LINK;
            out.param_4.label = GetContext()->GetLabelTo(in.param_4);
        }
        else
            BOOST_ASSERT(false);
        break;

    default:
        BOOST_ASSERT(false);
    }
}

void InstructionItem::ConvertParam48(Param48& out, uint32_t in)
{
    switch (Parameter::TypeTag(in))
    {
    case Parameter::Type48::MEM_OFFSET:
        out.type = Param48::MEM_OFFSET;
        out.label = GetContext()->GetLabelTo(Parameter::Value(in));
        break;
    case Parameter::Type48::IMMEDIATE:
        out.type = Param48::IMMEDIATE;
        out.num = Parameter::Value(in);
        break;
    case Parameter::Type48::INDIRECT:
        out.type = Param48::INDIRECT;
        out.num = Parameter::Value(in);
        break;
    case Parameter::Type48::SPECIAL:
        if (in >= Parameter::Type48Special::READ_STACK_MIN &&
            in <= Parameter::Type48Special::READ_STACK_MAX)
        {
            out.type = Param48::READ_STACK;
            out.num = in - Parameter::Type48Special::READ_STACK_MIN;
        }
        else if (in >= Parameter::Type48Special::READ_4AC_MIN &&
                 in <= Parameter::Type48Special::READ_4AC_MAX)
        {
            out.type = Param48::READ_4AC;
            out.num = in - Parameter::Type48Special::READ_4AC_MIN;
        }
        else
            BOOST_ASSERT(false);
        break;

    default:
        BOOST_ASSERT(false);
    }
}

static const std::set<uint32_t> no_returns{0, 6};

InstructionItem* InstructionItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    if (x.src.GetSize() < sizeof(Instruction))
        throw std::runtime_error("Invalid instruction: premature end of data");

    auto inst = x.src.Pread<Instruction>(0);
    if (x.src.GetSize() < inst.size)
        throw std::runtime_error("Invalid instruction: premature end of data");

    auto ret = x.ritem.SplitCreate<InstructionItem>(ptr.offset, x.src);

    auto rem_data = inst.size - sizeof(Instruction) -
        sizeof(Parameter) * inst.param_count;
    if (rem_data)
        ret->PrependChild(asserted_cast<RawItem*>(
            ret->GetNext())->Split(0, rem_data)->Remove());

    BOOST_ASSERT(ret->GetSize() == inst.size);

    // recursive parse
    if (ret->is_call)
        MaybeCreate<InstructionItem>(ret->target->second);
    if (ret->is_call || !no_returns.count(ret->opcode))
        MaybeCreate<InstructionItem>({ret->GetNext(), 0});
    for (const auto& p : ret->params)
    {
        if (p.type == InstructionItem::Param::MEM_OFFSET)
            MaybeCreate<DataItem>(p.param_0.label->second);
        else if (p.type == InstructionItem::Param::INSTR_PTR0 ||
            p.type == InstructionItem::Param::INSTR_PTR1)
            MaybeCreate<InstructionItem>(p.param_4.label->second);
    }

    return ret;
}


FilePosition InstructionItem::GetSize() const noexcept
{
    return sizeof(Instruction) + params.size() * sizeof(Parameter) +
        ItemWithChildren::GetSize();
}

void InstructionItem::Dump48(
    boost::endian::little_uint32_t& out, const Param48& in) const noexcept
{
    switch (in.type)
    {
    case Param48::MEM_OFFSET:
        out = Parameter::Tag(Parameter::Type48::MEM_OFFSET, ToFilePos(in.label->second));
        return;
    case Param48::IMMEDIATE:
        out = Parameter::Tag(Parameter::Type48::IMMEDIATE, in.num);
        return;
    case Param48::INDIRECT:
        out = Parameter::Tag(Parameter::Type48::INDIRECT, in.num);
        return;
    case Param48::READ_STACK:
        out = Parameter::Type48Special::READ_STACK_MIN + in.num;
        return;
    case Param48::READ_4AC:
        out = Parameter::Type48Special::READ_4AC_MIN + in.num;
        return;
    }
    BOOST_ASSERT(false);
}

FilePosition InstructionItem::UpdatePositions(FilePosition npos)
{
    if (GetChildren())
        GetChildren()->UpdatePositions(
            npos + sizeof(Instruction) + params.size() * sizeof(Parameter));
    return Item::UpdatePositions(npos);
}

void InstructionItem::Dump(std::ostream& os) const
{
    Instruction ins;
    ins.is_call = is_call;

    if (is_call)
        ins.opcode = ToFilePos(target->second);
    else
        ins.opcode = opcode;
    ins.param_count = params.size();
    ins.size = GetSize();
    os.write(reinterpret_cast<char*>(&ins), sizeof(Instruction));

    Parameter pp;
    for (const auto& p : params)
    {
        switch (p.type)
        {
        case Param::MEM_OFFSET:
            pp.param_0 =
                Parameter::Tag(Parameter::Type0::MEM_OFFSET, ToFilePos(p.param_0.label->second));
            Dump48(pp.param_4, p.param_4);
            Dump48(pp.param_8, p.param_8);
            break;

        case Param::INDIRECT:
            pp.param_0 = Parameter::Tag(Parameter::Type0::INDIRECT, p.param_0.num);
            pp.param_4 = 0x40000000;
            Dump48(pp.param_8, p.param_8);
            break;

        case Param::READ_STACK:
            pp.param_0 = Parameter::Type0Special::READ_STACK_MIN + p.param_0.num;
            pp.param_4 = 0x40000000;
            pp.param_8 = 0x40000000;
            break;

        case Param::READ_4AC:
            pp.param_0 = Parameter::Type0Special::READ_4AC_MIN + p.param_0.num;
            pp.param_4 = 0x40000000;
            pp.param_8 = 0x40000000;
            break;

        case Param::INSTR_PTR0:
            pp.param_0 = Parameter::Type0Special::INSTR_PTR0;
            pp.param_4 = ToFilePos(p.param_4.label->second);
            pp.param_8 = 0x40000000;
            break;

        case Param::INSTR_PTR1:
            pp.param_0 = Parameter::Type0Special::INSTR_PTR1;
            pp.param_4 = ToFilePos(p.param_4.label->second);
            pp.param_8 = 0x40000000;
            break;

        case Param::COLL_LINK:
            pp.param_0 = Parameter::Type0Special::COLL_LINK;
            pp.param_4 = ToFilePos(p.param_4.label->second);
            pp.param_8 = 0;
            break;
        }
        os.write(reinterpret_cast<char*>(&pp), sizeof(Parameter));
    }

    ItemWithChildren::Dump(os);
}

void InstructionItem::PrettyPrint(std::ostream &os) const
{
    Item::PrettyPrint(os);

    if (is_call)
        os << "call @" << target->first;
    else
        os << "instr " << opcode;
    os << '(';
    bool sep = false;
    for (const auto& p : params)
    {
        if (sep) os << ", ";
        sep = true;
        os << p;
    }
    os << ") {";
    if (GetChildren()) os << '\n' << *GetChildren();
    os << "}\n";
}

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p)
{
    switch (p.type)
    {
    case InstructionItem::Param48::MEM_OFFSET:
        return os << "@" << p.label->first;
    case InstructionItem::Param48::IMMEDIATE:
        return os << p.num;
    case InstructionItem::Param48::INDIRECT:
        return os << "indirect(" << p.num << ')';
    case InstructionItem::Param48::READ_STACK:
        return os << "stack(" << p.num << ')';
    case InstructionItem::Param48::READ_4AC:
        return os << "4ac(" << p.num << ')';
    }
    abort();
}
std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p)
{
    switch (p.type)
    {
    case InstructionItem::Param::MEM_OFFSET:
        return os << "mem_offset(@" << p.param_0.label->first << ", "
                  << p.param_4 << ", " << p.param_8 << ')';
    case InstructionItem::Param::INDIRECT:
        return os << "indirect(" << p.param_0.num << ", " << p.param_8 << ')';
    case InstructionItem::Param::READ_STACK:
        return os << "stack(" << p.param_0.num << ")";
    case InstructionItem::Param::READ_4AC:
        return os << "4ac(" << p.param_0.num << ")";
    case InstructionItem::Param::INSTR_PTR0:
        return os << "instr_ptr0(@" << p.param_4.label->first << ')';
    case InstructionItem::Param::INSTR_PTR1:
        return os << "instr_ptr1(@" << p.param_4.label->first << ')';
    case InstructionItem::Param::COLL_LINK:
        return os << "coll_link(@" << p.param_4.label->first << ')';
    }
    abort();
}

}
