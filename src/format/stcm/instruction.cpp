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
    instr.Validate(GetContext().GetSize());

    is_call = instr.is_call;
    if (is_call)
        target = &GetContext().GetLabelTo(instr.opcode);
    else
        opcode = instr.opcode;

    params.resize(instr.param_count);
    for (size_t i = 0; i < instr.param_count; ++i)
    {
        auto p = src.ReadGen<Parameter>();
        p.Validate(GetContext().GetSize());
        ConvertParam(params[i], p);
    }
}

void InstructionItem::ConvertParam(Param& out, const Parameter& in)
{
    switch (Parameter::TypeTag(in.param_0))
    {
    case Parameter::Type0::MEM_OFFSET:
        out.type = Param::MEM_OFFSET;
        out.param_0.label = &GetContext().GetLabelTo(
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
            out.param_4.label = &GetContext().GetLabelTo(in.param_4);
        }
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR1)
        {
            out.type = Param::INSTR_PTR1;
            out.param_4.label = &GetContext().GetLabelTo(in.param_4);
        }
        else if (in.param_0 == Parameter::Type0Special::COLL_LINK)
        {
            out.type = Param::COLL_LINK;
            out.param_4.label = &GetContext().GetLabelTo(in.param_4);
        }
        else
            NEPTOOLS_UNREACHABLE("Invalid special parameter type");
        break;

    default:
        NEPTOOLS_UNREACHABLE("Invalid parameter type");
    }
}

void InstructionItem::ConvertParam48(Param48& out, uint32_t in)
{
    switch (Parameter::TypeTag(in))
    {
    case Parameter::Type48::MEM_OFFSET:
        out.type = Param48::MEM_OFFSET;
        out.label = &GetContext().GetLabelTo(Parameter::Value(in));
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
            NEPTOOLS_UNREACHABLE("Invalid 48Special param");
        break;

    default:
        NEPTOOLS_UNREACHABLE("Invalid 48 param");
    }
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
        if (p.type == InstructionItem::Param::MEM_OFFSET)
            MaybeCreate<DataItem>(p.param_0.label->ptr);
        else if (p.type == InstructionItem::Param::INSTR_PTR0 ||
            p.type == InstructionItem::Param::INSTR_PTR1)
            MaybeCreate<InstructionItem>(p.param_4.label->ptr);
    }

    return ret;
}


FilePosition InstructionItem::GetSize() const noexcept
{
    return sizeof(Header) + params.size() * sizeof(Parameter) +
        ItemWithChildren::GetSize();
}

void InstructionItem::Dump48(
    boost::endian::little_uint32_t& out, const Param48& in) const noexcept
{
    switch (in.type)
    {
    case Param48::MEM_OFFSET:
        out = Parameter::Tag(Parameter::Type48::MEM_OFFSET, ToFilePos(in.label->ptr));
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
    NEPTOOLS_UNREACHABLE("Invalid Param48 Type stored");
}

void InstructionItem::Fixup()
{
    ItemWithChildren::Fixup_(sizeof(Header) + params.size() * sizeof(Parameter));
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

    Parameter pp;
    for (const auto& p : params)
    {
        switch (p.type)
        {
        case Param::MEM_OFFSET:
            pp.param_0 =
                Parameter::Tag(Parameter::Type0::MEM_OFFSET, ToFilePos(p.param_0.label->ptr));
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
            pp.param_4 = ToFilePos(p.param_4.label->ptr);
            pp.param_8 = 0x40000000;
            break;

        case Param::INSTR_PTR1:
            pp.param_0 = Parameter::Type0Special::INSTR_PTR1;
            pp.param_4 = ToFilePos(p.param_4.label->ptr);
            pp.param_8 = 0x40000000;
            break;

        case Param::COLL_LINK:
            pp.param_0 = Parameter::Type0Special::COLL_LINK;
            pp.param_4 = ToFilePos(p.param_4.label->ptr);
            pp.param_8 = 0;
            break;
        }
        sink.WriteGen(pp);
    }

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

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p)
{
    switch (p.type)
    {
    case InstructionItem::Param48::MEM_OFFSET:
        return os << "@" << p.label->name;
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
        return os << "mem_offset(@" << p.param_0.label->name << ", "
                  << p.param_4 << ", " << p.param_8 << ')';
    case InstructionItem::Param::INDIRECT:
        return os << "indirect(" << p.param_0.num << ", " << p.param_8 << ')';
    case InstructionItem::Param::READ_STACK:
        return os << "stack(" << p.param_0.num << ")";
    case InstructionItem::Param::READ_4AC:
        return os << "4ac(" << p.param_0.num << ")";
    case InstructionItem::Param::INSTR_PTR0:
        return os << "instr_ptr0(@" << p.param_4.label->name << ')';
    case InstructionItem::Param::INSTR_PTR1:
        return os << "instr_ptr1(@" << p.param_4.label->name << ')';
    case InstructionItem::Param::COLL_LINK:
        return os << "coll_link(@" << p.param_4.label->name << ')';
    }
    abort();
}

}
}
