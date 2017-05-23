#include "instruction.hpp"
#include "data.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../except.hpp"
#include "../../sink.hpp"
#include <set>
#include <iostream>

namespace Neptools
{
namespace Stcm
{

#define IP InstructionItem::Parameter

static void Param48Validate(uint32_t param, FilePosition file_size)
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm Param48", x)
    switch (IP::TypeTag(param))
    {
    case IP::Type48::MEM_OFFSET:
        VALIDATE((IP::Value(param) + 16) < file_size);
        return;
    case IP::Type48::IMMEDIATE:
        return;
    case IP::Type48::INDIRECT:
        return; // ??
    default:
        VALIDATE((param >= IP::Type48Special::READ_STACK_MIN &&
                  param <= IP::Type48Special::READ_STACK_MAX) ||
                 (param >= IP::Type48Special::READ_4AC_MIN &&
                  param <= IP::Type48Special::READ_4AC_MAX));
        return;
    }
#undef VALIDATE
}

void InstructionItem::Parameter::Validate(FilePosition file_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm::InstructionItem::Parameter", x)
    switch (TypeTag(param_0))
    {
    case Type0::MEM_OFFSET:
        VALIDATE(Value(param_0) < file_size);
        Param48Validate(param_4, file_size);
        Param48Validate(param_8, file_size);
        return;

    //case Type0::UNK: todo
    case Type0::INDIRECT:
        VALIDATE(Value(param_0) < 256 && param_4 == 0x40000000);
        Param48Validate(param_8, file_size);
        return;

    case Type0::SPECIAL:
        if ((param_0 >= Type0Special::READ_STACK_MIN && param_0 <= Type0Special::READ_STACK_MAX) ||
            (param_0 >= Type0Special::READ_4AC_MIN && param_0 <= Type0Special::READ_4AC_MAX))
        {
            VALIDATE(param_4 == 0x40000000);
            VALIDATE(param_8 == 0x40000000);
        }
        else if (param_0 == Type0Special::INSTR_PTR0 ||
                 param_0 == Type0Special::INSTR_PTR1)
        {
            VALIDATE(param_4 < file_size);
            VALIDATE(param_8 == 0x40000000);
        }
        else if (param_0 == Type0Special::COLL_LINK)
        {
            VALIDATE(param_4 + 8 < file_size);
            VALIDATE(param_8 == 0);
        }
        else
            VALIDATE(!"Unknown special");
        return;
    default:
        VALIDATE(!"Unknown type0");
    }
#undef VALIDATE
}

void InstructionItem::Header::Validate(FilePosition file_size) const
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stcm::InstructionItem::Header", x)
    VALIDATE(is_call == 0 || is_call == 1);
    VALIDATE(param_count < 16);
    VALIDATE(size >= sizeof(Header) + param_count*sizeof(Parameter));

    if (is_call)
        VALIDATE(opcode < file_size);
    else
        VALIDATE(opcode < USER_OPCODES ||
                 (opcode >= SYSTEM_OPCODES_BEGIN && opcode <= SYSTEM_OPCODES_END));
#undef VALIDATE
}

InstructionItem::InstructionItem(Key k, Context* ctx, Source src)
    : ItemWithChildren{k, ctx}
{
    AddInfo(&InstructionItem::Parse_, ADD_SOURCE(src), this, src);
}

void InstructionItem::Parse_(Source& src)
{
    auto instr = src.ReadGen<Header>();
    auto& ctx = GetUnsafeContext();
    instr.Validate(ctx.GetSize());

    is_call = instr.is_call;
    if (instr.is_call)
        SetTarget(&ctx.GetLabelTo(instr.opcode));
    else
        SetOpcode(instr.opcode);

    params.reserve(instr.param_count);
    for (size_t i = 0; i < instr.param_count; ++i)
    {
        auto p = src.ReadGen<Parameter>();
        params.emplace_back(ctx, p);
    }
}

InstructionItem::Param::Param(Context& ctx, const Parameter& in)
{
    in.Validate(ctx.GetSize());

    switch (Parameter::TypeTag(in.param_0))
    {
    case Parameter::Type0::MEM_OFFSET:
        SetMemOffset(ctx.GetLabelTo(Parameter::Value(in.param_0)));
        param_4 = Param48{ctx, in.param_4};
        param_8 = Param48{ctx, in.param_8};
        return;

    case Parameter::Type0::INDIRECT:
        SetIndirect(Parameter::Value(in.param_0));
        param_8 = Param48{ctx, in.param_8};
        return;

    case Parameter::Type0::SPECIAL:
        if (in.param_0 >= Parameter::Type0Special::READ_STACK_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_STACK_MAX)
        {
            SetReadStack(in.param_0 - Parameter::Type0Special::READ_STACK_MIN);
        }
        else if (in.param_0 >= Parameter::Type0Special::READ_4AC_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_4AC_MAX)
        {
            SetRead4ac(in.param_0 - Parameter::Type0Special::READ_4AC_MIN);
        }
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR0)
            SetInstrPtr0(ctx.GetLabelTo(in.param_4));
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR1)
            SetInstrPtr1(ctx.GetLabelTo(in.param_4));
        else if (in.param_0 == Parameter::Type0Special::COLL_LINK)
            SetCollLink(ctx.GetLabelTo(in.param_4));
        else
            NEPTOOLS_UNREACHABLE("Invalid special parameter type");
        return;
    }

    NEPTOOLS_UNREACHABLE("Invalid parameter type");
}

void InstructionItem::Param::Dump(Sink& sink) const
{
    Parameter pp;
    switch (GetType())
    {
    case Type::MEM_OFFSET:
        pp.param_0 =
            Parameter::Tag(Parameter::Type0::MEM_OFFSET, ToFilePos(label->ptr));
        pp.param_4 = param_4.Dump();
        pp.param_8 = param_8.Dump();
        break;

    case Type::INDIRECT:
        pp.param_0 = Parameter::Tag(Parameter::Type0::INDIRECT, num);
        pp.param_4 = 0x40000000;
        pp.param_8 = param_8.Dump();
        break;

    case Type::READ_STACK:
        pp.param_0 = Parameter::Type0Special::READ_STACK_MIN + num;
        pp.param_4 = 0x40000000;
        pp.param_8 = 0x40000000;
        break;

    case Type::READ_4AC:
        pp.param_0 = Parameter::Type0Special::READ_4AC_MIN + num;
        pp.param_4 = 0x40000000;
        pp.param_8 = 0x40000000;
        break;

    case Type::INSTR_PTR0:
        pp.param_0 = Parameter::Type0Special::INSTR_PTR0;
        pp.param_4 = ToFilePos(label->ptr);
        pp.param_8 = 0x40000000;
        break;

    case Type::INSTR_PTR1:
        pp.param_0 = Parameter::Type0Special::INSTR_PTR1;
        pp.param_4 = ToFilePos(label->ptr);
        pp.param_8 = 0x40000000;
        break;

    case Type::COLL_LINK:
        pp.param_0 = Parameter::Type0Special::COLL_LINK;
        pp.param_4 = ToFilePos(label->ptr);
        pp.param_8 = 0;
        break;
    }
    sink.WriteGen(pp);
}

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p)
{
    switch (p.GetType())
    {
    case InstructionItem::Param::Type::MEM_OFFSET:
        return os << "mem_offset(@" << p.GetMemOffset().name << ", "
                  << p.GetParam4() << ", " << p.GetParam8() << ')';
    case InstructionItem::Param::Type::INDIRECT:
        return os << "indirect(" << p.GetIndirect() << ", " << p.GetParam8() << ')';
    case InstructionItem::Param::Type::READ_STACK:
        return os << "stack(" << p.GetReadStack() << ")";
    case InstructionItem::Param::Type::READ_4AC:
        return os << "4ac(" << p.GetRead4ac() << ")";
    case InstructionItem::Param::Type::INSTR_PTR0:
        return os << "instr_ptr0(@" << p.GetInstrPtr0().name << ')';
    case InstructionItem::Param::Type::INSTR_PTR1:
        return os << "instr_ptr1(@" << p.GetInstrPtr1().name << ')';
    case InstructionItem::Param::Type::COLL_LINK:
        return os << "coll_link(@" << p.GetCollLink().name << ')';
    }
    NEPTOOLS_UNREACHABLE("Invalid type");
}


InstructionItem::Param48::Param48(Context& ctx, uint32_t in)
{
    switch (Parameter::TypeTag(in))
    {
    case Parameter::Type48::MEM_OFFSET:
        SetMemOffset(ctx.GetLabelTo(Parameter::Value(in)));
        return;

    case Parameter::Type48::IMMEDIATE:
        SetImmediate(Parameter::Value(in));
        return;

    case Parameter::Type48::INDIRECT:
        SetIndirect(Parameter::Value(in));
        return;

    case Parameter::Type48::SPECIAL:
        if (in >= Parameter::Type48Special::READ_STACK_MIN &&
            in <= Parameter::Type48Special::READ_STACK_MAX)
        {
            SetReadStack(in - Parameter::Type48Special::READ_STACK_MIN);
        }
        else if (in >= Parameter::Type48Special::READ_4AC_MIN &&
                 in <= Parameter::Type48Special::READ_4AC_MAX)
        {
            SetRead4ac(in - Parameter::Type48Special::READ_4AC_MIN);
        }
        else
            NEPTOOLS_UNREACHABLE("Invalid 48Special param");
        return;
    }

    NEPTOOLS_UNREACHABLE("Invalid 48 param");
}

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p)
{
    switch (p.GetType())
    {
    case InstructionItem::Param48::Type::MEM_OFFSET:
        return os << "mem_offset(@" << p.GetMemOffset().name << ')';
    case InstructionItem::Param48::Type::IMMEDIATE:
        return os << "immediate(" << p.GetImmediate() << ')';
    case InstructionItem::Param48::Type::INDIRECT:
        return os << "indirect(" << p.GetIndirect() << ')';
    case InstructionItem::Param48::Type::READ_STACK:
        return os << "read_stack(" << p.GetReadStack() << ')';
    case InstructionItem::Param48::Type::READ_4AC:
        return os << "read_4ac(" << p.GetRead4ac() << ')';
    }

    NEPTOOLS_UNREACHABLE("Invalid 48 param");
}

static const std::set<uint32_t> no_returns{0, 6};

InstructionItem& InstructionItem::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);

    x.src.CheckSize(sizeof(Header));
    auto inst = x.src.PreadGen<Header>(0);
    x.src.CheckSize(inst.size);

    auto& ret = x.ritem.SplitCreate<InstructionItem>(ptr.offset, x.src);

    auto rem_data = inst.size - sizeof(Header) -
        sizeof(Parameter) * inst.param_count;
    if (rem_data)
        ret.MoveNextToChild(rem_data);

    NEPTOOLS_ASSERT(ret.GetSize() == inst.size);

    // recursive parse
    if (ret.is_call)
        MaybeCreate<InstructionItem>(ret.target->ptr);
    if (ret.is_call || !no_returns.count(ret.opcode))
        MaybeCreate<InstructionItem>({&*++ret.Iterator(), 0});
    for (const auto& p : ret.params)
    {
        if (p.GetType() == Param::Type::MEM_OFFSET)
            MaybeCreate<DataItem>(p.GetMemOffset().ptr);
        else if (p.GetType() == Param::Type::INSTR_PTR0 ||
                 p.GetType() == Param::Type::INSTR_PTR1)
            MaybeCreate<InstructionItem>(p.GetLabel().ptr);
    }

    return ret;
}


FilePosition InstructionItem::GetSize() const noexcept
{
    return sizeof(Header) + params.size() * sizeof(Parameter) +
        ItemWithChildren::GetSize();
}

uint32_t InstructionItem::Param48::Dump() const noexcept
{
    switch (GetType())
    {
    case Type::MEM_OFFSET:
        return Parameter::Tag(
            Parameter::Type48::MEM_OFFSET, ToFilePos(label->ptr));
    case Type::IMMEDIATE:
        return Parameter::Tag(Parameter::Type48::IMMEDIATE, num);
    case Type::INDIRECT:
        return Parameter::Tag(Parameter::Type48::INDIRECT, num);
    case Type::READ_STACK:
        return Parameter::Type48Special::READ_STACK_MIN + num;
    case Type::READ_4AC:
        return Parameter::Type48Special::READ_4AC_MIN + num;
    }
    NEPTOOLS_UNREACHABLE("Invalid Param48 Type stored");
}

void InstructionItem::Fixup()
{
    ItemWithChildren::Fixup_(sizeof(Header) + params.size() * sizeof(Parameter));
}

void InstructionItem::Dispose() noexcept
{
    params.clear();
    ItemWithChildren::Dispose();
}

void InstructionItem::Dump_(Sink& sink) const
{
    Header hdr;
    hdr.is_call = is_call;

    if (is_call)
        hdr.opcode = ToFilePos(target->ptr);
    else
        hdr.opcode = opcode;
    hdr.param_count = params.size();
    hdr.size = GetSize();
    sink.WriteGen(hdr);

    for (const auto& p : params)
        p.Dump(sink);

    ItemWithChildren::Dump_(sink);
}

void InstructionItem::Inspect_(std::ostream &os) const
{
    Item::Inspect_(os);

    if (is_call)
        os << "call @" << target->name;
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
    ItemWithChildren::Inspect_(os);
    os << "}\n";
}

}
}
