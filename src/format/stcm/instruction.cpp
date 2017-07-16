#include "instruction.hpp"
#include "data.hpp"
#include "../context.hpp"
#include "../raw_item.hpp"
#include "../../container/vector.lua.hpp"
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

InstructionItem::InstructionItem(Key k, Context& ctx, Source src)
    : ItemWithChildren{k, ctx}
{
    AddInfo(&InstructionItem::Parse_, ADD_SOURCE(src), this, ctx, src);
}

void InstructionItem::Parse_(Context& ctx, Source& src)
{
    auto instr = src.ReadGen<Header>();
    instr.Validate(ctx.GetSize());

    if (instr.is_call)
        SetTarget(ctx.GetLabelTo(instr.opcode));
    else
        SetOpcode(instr.opcode);

    params.reserve(instr.param_count);
    for (size_t i = 0; i < instr.param_count; ++i)
    {
        auto p = src.ReadGen<Parameter>();
        params.emplace_back(ctx, p);
    }
}

#ifndef NEPTOOLS_WITHOUT_LUA
void InstructionItem::ParseArgs(Lua::StateRef vm, Lua::RawTable tbl)
{
    Lua::Vector<Param>::FillFromTable(vm, params, tbl);
}
#endif

auto InstructionItem::Param::GetVariant(Context& ctx, const Parameter& in)
    -> Variant
{
    in.Validate(ctx.GetSize());

    switch (Parameter::TypeTag(in.param_0))
    {
    case Parameter::Type0::MEM_OFFSET:
        return MemOffset{
            ctx.GetLabelTo(Parameter::Value(in.param_0)),
            Param48{ctx, in.param_4},
            Param48{ctx, in.param_8}};

    case Parameter::Type0::INDIRECT:
        return Indirect{
            Parameter::Value(in.param_0),
            Param48{ctx, in.param_8}};

#define RETVAR(type, val) return Variant{                               \
            std::in_place_index<static_cast<size_t>(Type::type)>, val}
    case Parameter::Type0::SPECIAL:
        if (in.param_0 >= Parameter::Type0Special::READ_STACK_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_STACK_MAX)
        {
            RETVAR(READ_STACK, in.param_0 - Parameter::Type0Special::READ_STACK_MIN);
        }
        else if (in.param_0 >= Parameter::Type0Special::READ_4AC_MIN &&
            in.param_0 <= Parameter::Type0Special::READ_4AC_MAX)
        {
            RETVAR(READ_4AC, in.param_0 - Parameter::Type0Special::READ_4AC_MIN);
        }
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR0)
            RETVAR(INSTR_PTR0, ctx.GetLabelTo(in.param_4));
        else if (in.param_0 == Parameter::Type0Special::INSTR_PTR1)
            RETVAR(INSTR_PTR1, ctx.GetLabelTo(in.param_4));
        else if (in.param_0 == Parameter::Type0Special::COLL_LINK)
            RETVAR(COLL_LINK, ctx.GetLabelTo(in.param_4));
        else
            NEPTOOLS_UNREACHABLE("Invalid special parameter type");
    }

    NEPTOOLS_UNREACHABLE("Invalid parameter type");
}

void InstructionItem::Param::Dump(Sink& sink) const
{
    Parameter pp;
    switch (GetType())
    {
    case Type::MEM_OFFSET:
    {
        const auto& o = Get<Type::MEM_OFFSET>();
        pp.param_0 = Parameter::Tag(
            Parameter::Type0::MEM_OFFSET, ToFilePos(o.target->GetPtr()));
        pp.param_4 = o.param_4.Dump();
        pp.param_8 = o.param_8.Dump();
        break;
    }

    case Type::INDIRECT:
    {
        const auto& i = Get<Type::INDIRECT>();
        pp.param_0 = Parameter::Tag(Parameter::Type0::INDIRECT, i.param_0);
        pp.param_4 = 0x40000000;
        pp.param_8 = i.param_8.Dump();
        break;
    }

    case Type::READ_STACK:
        pp.param_0 = Parameter::Type0Special::READ_STACK_MIN +
            Get<Type::READ_STACK>();
        pp.param_4 = 0x40000000;
        pp.param_8 = 0x40000000;
        break;

    case Type::READ_4AC:
        pp.param_0 = Parameter::Type0Special::READ_4AC_MIN +
            Get<Type::READ_4AC>();
        pp.param_4 = 0x40000000;
        pp.param_8 = 0x40000000;
        break;

    case Type::INSTR_PTR0:
        pp.param_0 = Parameter::Type0Special::INSTR_PTR0;
        pp.param_4 = ToFilePos(Get<Type::INSTR_PTR0>()->GetPtr());
        pp.param_8 = 0x40000000;
        break;

    case Type::INSTR_PTR1:
        pp.param_0 = Parameter::Type0Special::INSTR_PTR1;
        pp.param_4 = ToFilePos(Get<Type::INSTR_PTR1>()->GetPtr());
        pp.param_8 = 0x40000000;
        break;

    case Type::COLL_LINK:
        pp.param_0 = Parameter::Type0Special::COLL_LINK;
        pp.param_4 = ToFilePos(Get<Type::COLL_LINK>()->GetPtr());
        pp.param_8 = 0;
        break;
    }
    sink.WriteGen(pp);
}

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param& p)
{
    os << "neptools.stcm.instruction_item.param.new_";
    using T = InstructionItem::Param::Type;
    switch (p.GetType())
    {
    case T::MEM_OFFSET:
    {
        const auto& o = p.Get<T::MEM_OFFSET>();
        return os << "mem_offset(" << PrintLabel(o.target) << ", "
                  << o.param_4 << ", " << o.param_8 << ')';
    }
    case T::INDIRECT:
    {
        const auto& i = p.Get<T::INDIRECT>();
        return os << "indirect(" << i.param_0 << ", " << i.param_8 << ')';
    }
    case T::READ_STACK:
        return os << "read_stack(" << p.Get<T::READ_STACK>() << ")";
    case T::READ_4AC:
        return os << "read_4ac(" << p.Get<T::READ_4AC>() << ")";
    case T::INSTR_PTR0:
        return os << "instr_ptr0(" << PrintLabel(p.Get<T::INSTR_PTR0>()) << ')';
    case T::INSTR_PTR1:
        return os << "instr_ptr1(" << PrintLabel(p.Get<T::INSTR_PTR1>()) << ')';
    case T::COLL_LINK:
        return os << "coll_link(" << PrintLabel(p.Get<T::COLL_LINK>()) << ')';
    }
    NEPTOOLS_UNREACHABLE("Invalid type");
}


auto InstructionItem::Param48::GetVariant(Context& ctx, uint32_t in) -> Variant
{
    switch (Parameter::TypeTag(in))
    {
    case Parameter::Type48::MEM_OFFSET:
        RETVAR(MEM_OFFSET, ctx.GetLabelTo(Parameter::Value(in)));

    case Parameter::Type48::IMMEDIATE:
        RETVAR(IMMEDIATE, Parameter::Value(in));

    case Parameter::Type48::INDIRECT:
        RETVAR(INDIRECT, Parameter::Value(in));

    case Parameter::Type48::SPECIAL:
        if (in >= Parameter::Type48Special::READ_STACK_MIN &&
            in <= Parameter::Type48Special::READ_STACK_MAX)
        {
            RETVAR(READ_STACK, in - Parameter::Type48Special::READ_STACK_MIN);
        }
        else if (in >= Parameter::Type48Special::READ_4AC_MIN &&
                 in <= Parameter::Type48Special::READ_4AC_MAX)
        {
            RETVAR(READ_4AC, in - Parameter::Type48Special::READ_4AC_MIN);
        }
        else
            NEPTOOLS_UNREACHABLE("Invalid 48Special param");
    }
#undef RETVAR

    NEPTOOLS_UNREACHABLE("Invalid 48 param");
}

std::ostream& operator<<(std::ostream& os, const InstructionItem::Param48& p)
{
    os << "neptools.stcm.instruction_item.param48.new_";
    using T = InstructionItem::Param48::Type;
    switch (p.GetType())
    {
    case T::MEM_OFFSET:
        return os << "mem_offset(" << PrintLabel(p.Get<T::MEM_OFFSET>()) << ')';
    case T::IMMEDIATE:
        return os << "immediate(" << p.Get<T::IMMEDIATE>() << ')';
    case T::INDIRECT:
        return os << "indirect(" << p.Get<T::INDIRECT>() << ')';
    case T::READ_STACK:
        return os << "read_stack(" << p.Get<T::READ_STACK>() << ')';
    case T::READ_4AC:
        return os << "read_4ac(" << p.Get<T::READ_4AC>() << ')';
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
    if (ret.IsCall())
        MaybeCreate<InstructionItem>(ret.GetTarget()->GetPtr());
    if (ret.IsCall() || !no_returns.count(ret.GetOpcode()))
        MaybeCreate<InstructionItem>({&*++ret.Iterator(), 0});
    for (const auto& p : ret.params)
    {
        using T = Param::Type;
        switch (p.GetType())
        {
        case T::MEM_OFFSET:
            MaybeCreate<DataItem>(p.Get<T::MEM_OFFSET>().target->GetPtr());
            break;
        case T::INSTR_PTR0:
            MaybeCreate<InstructionItem>(p.Get<T::INSTR_PTR0>()->GetPtr());
            break;
        case T::INSTR_PTR1:
            MaybeCreate<InstructionItem>(p.Get<T::INSTR_PTR1>()->GetPtr());
            break;
        default:;
        }
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
        return Parameter::Tag(Parameter::Type48::MEM_OFFSET,
                              ToFilePos(Get<Type::MEM_OFFSET>()->GetPtr()));
    case Type::IMMEDIATE:
        return Parameter::Tag(Parameter::Type48::IMMEDIATE, Get<Type::IMMEDIATE>());
    case Type::INDIRECT:
        return Parameter::Tag(Parameter::Type48::INDIRECT, Get<Type::INDIRECT>());
    case Type::READ_STACK:
        return Parameter::Type48Special::READ_STACK_MIN + Get<Type::READ_STACK>();
    case Type::READ_4AC:
        return Parameter::Type48Special::READ_4AC_MIN + Get<Type::READ_4AC>();
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
    hdr.is_call = IsCall();

    if (IsCall())
        hdr.opcode = ToFilePos(GetTarget()->GetPtr());
    else
        hdr.opcode = GetOpcode();
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

    if (IsCall())
        os << "call(" << PrintLabel(GetTarget());
    else
        os << "instruction(" << GetOpcode();
    os << ", {";
    bool sep = false;
    for (const auto& p : params)
    {
        if (sep) os << ", ";
        sep = true;
        os << p;
    }
    os << "})";
    InspectChildren(os);
}

}
}

NEPTOOLS_STD_VECTOR_LUAGEN(
    stcm_instruction_param, Neptools::Stcm::InstructionItem::Param);
#include "instruction.binding.hpp"
