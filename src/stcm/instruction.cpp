#include "instruction.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include <boost/assert.hpp>
#include <iostream>

#define IP Instruction::Parameter

namespace Stcm
{

static bool Param48Valid(uint32_t param, size_t file_size) noexcept
{
    switch (IP::TypeTag(param))
    {
    case IP::Type48::MEM_OFFSET:
        return (IP::Value(param) + 16) < file_size;
    case IP::Type48::IMMEDIATE:
        return true;
    case IP::Type48::INDIRECT:
        return true; // ??
    default:
        return false; // todo
    }
}

bool Instruction::Parameter::IsValid(size_t file_size) const noexcept
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
            return Param48Valid(param_4, file_size) && param_8 == 0x40000000;
        else
            return false;
    default:
        return false;
    }
}

bool Instruction::IsValid(size_t file_size) const noexcept
{
    if ((is_call != 0 && is_call != 1) ||
        param_count >= 16 ||
        size < Instruction::SIZE + param_count*sizeof(Parameter))
        return false;

    for (size_t i = 0; i < param_count; ++i)
        if (!params[i].IsValid(file_size))
            return false;

    if (is_call)
        return opcode < file_size;
    else
        return opcode < USER_OPCODES ||
            (opcode >= SYSTEM_OPCODES_BEGIN && opcode <= SYSTEM_OPCODES_END);
}

InstructionItem::InstructionItem(
    Key k, Context* ctx, RawItem& ritem, FilePosition offset)
    : Item{k, ctx}
{
    auto instr = reinterpret_cast<const Instruction*>(ritem.GetPtr() + offset);
    if (ritem.GetSize() - offset < Instruction::SIZE ||
        ritem.GetSize() - offset < instr->size)
        throw std::runtime_error("Invalid instruction: premature end of data");


    if (!instr->IsValid(ctx->GetSize()))
        throw std::runtime_error("invalid instruction");

    is_call = instr->is_call;
    if (is_call)
        target = ctx->GetLabelTo(instr->opcode);
    else
        opcode = instr->opcode;

    params.resize(instr->param_count);
    for (size_t i = 0; i < instr->param_count; ++i)
        ConvertParam(params[i], instr->params[i]);

    /*
    auto rem_data = instr->size - Instruction::SIZE -
        sizeof(Instruction::Parameter) * instr->param_count;
    std::cerr << instr->size << " " << rem_data << std::endl;
    if (rem_data)
        // todo: exception safety
        PrependChild(ritem.Split(instr->size - rem_data, rem_data)->Remove());

    BOOST_ASSERT(GetSize() == instr->size);
    */
}

void InstructionItem::ConvertParam(Param& out, const Instruction::Parameter& in)
{
    switch (IP::TypeTag(in.param_0))
    {
    case IP::Type0::MEM_OFFSET:
        out.type = Param::MEM_OFFSET;
        out.param_0.label = GetContext()->GetLabelTo(
            IP::Value(in.param_0));
        ConvertParam48(out.param_4, in.param_4);
        ConvertParam48(out.param_8, in.param_8);
        break;

    case IP::Type0::INDIRECT:
        out.type = Param::INDIRECT;
        out.param_0.num = IP::Value(in.param_0);
        ConvertParam48(out.param_8, in.param_8);
        break;

    case IP::Type0::SPECIAL:
        if (in.param_0 >= IP::Type0Special::READ_STACK_MIN &&
            in.param_0 <= IP::Type0Special::READ_STACK_MAX)
        {
            out.type = Param::READ_STACK;
            out.param_0.num = in.param_0 - IP::Type0Special::READ_STACK_MIN;
        }
        else if (in.param_0 >= IP::Type0Special::READ_4AC_MIN &&
            in.param_0 <= IP::Type0Special::READ_4AC_MAX)
        {
            out.type = Param::READ_4AC;
            out.param_0.num = in.param_0 - IP::Type0Special::READ_4AC_MIN;
        }
        else if (in.param_0 == IP::Type0Special::INSTR_PTR0)
        {
            out.type = Param::INSTR_PTR0;
            out.param_4.label = GetContext()->GetLabelTo(in.param_4);
        }
        else if (in.param_0 == IP::Type0Special::INSTR_PTR1)
        {
            out.type = Param::INSTR_PTR1;
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
    switch (IP::TypeTag(in))
    {
    case IP::Type48::MEM_OFFSET:
        out.type = Param48::MEM_OFFSET;
        out.label = GetContext()->GetLabelTo(
            IP::Value(in) + 16);
        break;
    case IP::Type48::IMMEDIATE:
        out.type = Param48::IMMEDIATE;
        out.num = IP::Value(in);
        break;
    case IP::Type48::INDIRECT:
        out.type = Param48::INDIRECT;
        out.num = IP::Value(in);
        break;

    default:
        BOOST_ASSERT(false);
    }
}

InstructionItem* InstructionItem::CreateAndInsert(Context* ctx, ItemPointer ptr)
{
    auto& ritem = dynamic_cast<RawItem&>(*ptr.item);
    auto instr = reinterpret_cast<const Instruction*>(ritem.GetPtr() + ptr.offset);
    auto ret = ritem.Split(ptr.offset, ctx->Create<InstructionItem>(ritem, ptr.offset));

    auto rem_data = instr->size - Instruction::SIZE -
        sizeof(Instruction::Parameter) * instr->param_count;
    if (rem_data)
        ret->PrependChild(reinterpret_cast<RawItem*>(
            ret->GetNext())->Split(0, rem_data)->Remove());

    BOOST_ASSERT(ret->GetSize() == instr->size);
    return ret;
}


size_t InstructionItem::GetSize() const noexcept
{
    size_t children_size = 0;
    for (auto p = GetChildren(); p; p = p->GetNext())
        children_size += p->GetSize();

    return Instruction::SIZE + params.size() * sizeof(Instruction::Parameter) +
        children_size;
}

void InstructionItem::Dump(std::ostream &os) const
{
    Item::Dump(os);

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

    default:
        abort();
    }
}


}
