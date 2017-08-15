#include "instruction.hpp"

#include "../cstring_item.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"
#include "../../lua/static_class.hpp"
#include "../../lua/user_type.hpp"

namespace Neptools
{
namespace Stsc
{

// helpers for generic CreateAndInsert
namespace
{
using CreateType = NotNull<SmartPtr<InstructionBase>>
    (*)(Context&, const Source&);

template <uint8_t I>
NotNull<SmartPtr<InstructionBase>>
CreateAdapt(Context& ctx, const Source& src)
{ return ctx.Create<InstructionItem<I>>(I, src); }

template <typename T> struct CreateMapImpl;
template <size_t... I>
struct CreateMapImpl<std::index_sequence<I...>>
{
    static const constexpr CreateType MAP[] = { CreateAdapt<I>... };
};

template <size_t... I>
const constexpr CreateType CreateMapImpl<std::index_sequence<I...>>::MAP[];

using CreateMap = CreateMapImpl<std::make_index_sequence<256>>;
}

// base
InstructionBase& InstructionBase::CreateAndInsert(ItemPointer ptr)
{
    auto x = RawItem::GetSource(ptr, -1);
    x.src.CheckSize(1);
    uint8_t opcode = x.src.ReadLittleUint8();
    auto ctx = x.ritem.GetContext();
    auto& ret = x.ritem.Split(ptr.offset, CreateMap::MAP[opcode](*ctx, x.src));

    ret.PostInsert();
    return ret;
}

void InstructionBase::InstrDump(Sink& sink) const
{
    sink.WriteLittleUint8(opcode);
}

std::ostream& InstructionBase::InstrInspect(std::ostream& os, unsigned indent) const
{
    Item::Inspect_(os, indent);
    auto flags = os.flags();
    os << "instruction(0x" << std::setw(2) << std::setfill('0') << std::hex
       << unsigned(opcode);
    os.flags(flags);
    return os;
}


// generic implementation
namespace
{

template <typename... Args> struct PODTuple;

template <typename Head, typename... Args>
struct PODTuple<Head, Args...>
{
    Head head;
    PODTuple<Args...> tail;
};

template <typename T> struct PODTuple<T> { T head; };
template<> struct PODTuple<> {};

template <size_t I> struct PODTupleGet
{
    template <typename T> static auto& Get(T& tuple)
    { return PODTupleGet<I-1>::Get(tuple.tail); }
};
template<> struct PODTupleGet<0>
{
    template <typename T> static auto& Get(T& tuple)
    { return tuple.head; }
};

template <size_t I, typename T> inline auto& Get(T& tuple)
{ return PODTupleGet<I>::Get(tuple); }


template <typename T> struct EndianMap;
template<> struct EndianMap<uint8_t>
{ using Type = boost::endian::little_uint8_t; };
template<> struct EndianMap<uint16_t>
{ using Type = boost::endian::little_uint16_t; };
template<> struct EndianMap<uint32_t>
{ using Type = boost::endian::little_uint32_t; };

template <typename T> using ToVoid = void;


template <typename T, typename Enable = void> struct Traits;

template <typename T>
struct Traits<T, ToVoid<typename EndianMap<T>::Type>>
{
    using RawType = typename EndianMap<T>::Type;
    static constexpr const size_t SIZE = sizeof(T);

    static void Validate(RawType, FilePosition) {}
    static T Parse(RawType r, Context&) { return r; }
    static RawType Dump(T r) { return r; }
    static void Inspect(std::ostream& os, T t) { os << uint32_t(t); }
    static void PostInsert(T) {}
};

template<> struct Traits<float>
{
    using RawType = boost::endian::little_uint32_t;
    static constexpr const size_t SIZE = 4;

    static void Validate(RawType, FilePosition) {}
    static float Parse(RawType r, Context&)
    {
        uint32_t num = r;
        float ret;
        memcpy(&ret, &num, sizeof(ret));
        return ret;
    }

    static RawType Dump(float f)
    {
        uint32_t ret;
        memcpy(&ret, &f, sizeof(ret));
        return ret;
    }

    static void Inspect(std::ostream& os, float v) { os << v; }
    static void PostInsert(float) {}
};

template<> struct Traits<void*>
{
    using RawType = boost::endian::little_uint32_t;
    static constexpr const size_t SIZE = 4;

    static void Validate(uint32_t r, FilePosition size)
    { NEPTOOLS_VALIDATE_FIELD("Stsc::Instruction", r <= size); }

    static NotNull<LabelPtr> Parse(uint32_t r, Context& ctx)
    { return ctx.GetLabelTo(r); }

    static RawType Dump(const LabelPtr& l)
    { return ToFilePos(l->GetPtr()); }

    static void Inspect(std::ostream& os, const LabelPtr& l)
    { os << PrintLabel(l); }

    static void PostInsert(const LabelPtr&) {}
};

template<> struct Traits<std::string> : public Traits<void*>
{
    static NotNull<LabelPtr> Parse(uint32_t r, Context& ctx)
    {
        auto ptr = ctx.GetPointer(r);
        if (ptr.Maybe<RawItem>())
        {
            auto x = RawItem::GetSource(ptr, -1);
            return ctx.GetLabelTo(
                r, CStringItem::GetLabelName(x.src.PreadCString(0)));
        }
        else
            return ctx.GetLabelTo(r);
    }

    static void PostInsert(const LabelPtr& lbl)
    { MaybeCreate<CStringItem>(lbl->GetPtr()); }
};

template<> struct Traits<Code*> : public Traits<void*>
{
    static void PostInsert(const LabelPtr& lbl)
    { MaybeCreateUnchecked<InstructionBase>(lbl->GetPtr()); }
};

template <typename T, typename... Args> struct OperationsImpl;
template <typename... T, size_t... I>
struct OperationsImpl<std::index_sequence<I...>, T...>
{
    using Swallow = int[];
#define FORALL(...) (void) Swallow{0, ((void)(__VA_ARGS__), 0)...}

    template <typename Tuple>
    static void Validate(const Tuple& tuple, FilePosition size)
    {
        (void) size; // shut up, retarded gcc
        FORALL(Traits<T>::Validate(Get<I>(tuple), size));
    }

    template <typename Dst, typename Src>
    static void Parse(Dst& dst, const Src& src, Context& ctx)
    {
        (void) ctx; // shut up, retarded gcc
        FORALL(std::get<I>(dst) = Traits<T>::Parse(Get<I>(src), ctx));
    }

    template <typename Dst, typename Src>
    static void Dump(Dst& dst, const Src& src)
    { FORALL(Get<I>(dst) = Traits<T>::Dump(std::get<I>(src))); }

    template <typename Tuple>
    static void Inspect(std::ostream& os, const Tuple& tuple)
    {
        FORALL(
            os << ", ",
            Traits<T>::Inspect(os, std::get<I>(tuple)));
    }

    template <typename Tuple>
    static void PostInsert(const Tuple& tuple)
    { FORALL(Traits<T>::PostInsert(std::get<I>(tuple))); }

    static constexpr size_t Size()
    {
        size_t sum = 0;
        FORALL(sum += Traits<T>::SIZE);
        return sum;
    }
#undef FORALL
};

template <typename... Args>
using Operations = OperationsImpl<std::index_sequence_for<Args...>, Args...>;

}

template <bool NoReturn, typename... Args>
const FilePosition SimpleInstruction<NoReturn, Args...>::SIZE =
    Operations<Args...>::Size() + 1;

template <bool NoReturn, typename... Args>
SimpleInstruction<NoReturn, Args...>::SimpleInstruction(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}
{
    AddInfo(&SimpleInstruction::Parse_, ADD_SOURCE(src), this, ctx, src);
}

template <bool NoReturn, typename... Args>
void SimpleInstruction<NoReturn, Args...>::Parse_(Context& ctx, Source& src)
{
    src.CheckSize(SIZE);
    using Tuple = PODTuple<typename Traits<Args>::RawType...>;
    NEPTOOLS_STATIC_ASSERT(std::is_pod<Tuple>::value);
    NEPTOOLS_STATIC_ASSERT(EmptySizeof<Tuple> == Operations<Args...>::Size());

    auto raw = src.ReadGen<Tuple>();

    Operations<Args...>::Validate(raw, ctx.GetSize());
    Operations<Args...>::Parse(args, raw, ctx);
}

template <bool NoReturn, typename... Args>
void SimpleInstruction<NoReturn, Args...>::Dump_(Sink& sink) const
{
    InstrDump(sink);

    using Tuple = PODTuple<typename Traits<Args>::RawType...>;
    Tuple t;
    Operations<Args...>::Dump(t, args);
    sink.WriteGen(t);
}

template <bool NoReturn, typename... Args>
void SimpleInstruction<NoReturn, Args...>::Inspect_(
    std::ostream& os, unsigned indent) const
{
    InstrInspect(os, indent);
    Operations<Args...>::Inspect(os, args);
    os << ')';
}

template <bool NoReturn, typename... Args>
void SimpleInstruction<NoReturn, Args...>::PostInsert()
{
    Operations<Args...>::PostInsert(args);
    if (!NoReturn) MaybeCreateUnchecked<InstructionBase>(&*++Iterator());
}

// ------------------------------------------------------------------------
// specific instruction implementations
Instruction0dItem::Instruction0dItem(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}
{
    AddInfo(&Instruction0dItem::Parse_, ADD_SOURCE(src), this, ctx, src);
}

void Instruction0dItem::Parse_(Context& ctx, Source& src)
{
    src.CheckRemainingSize(1);
    uint8_t n = src.ReadLittleUint8();
    src.CheckRemainingSize(4*n);

    tgts.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        uint32_t t = src.ReadLittleUint32();
        NEPTOOLS_VALIDATE_FIELD(
            "Stsc::Instruction0dItem", t < ctx.GetSize());
        tgts.push_back(ctx.GetLabelTo(t));
    }
}

void Instruction0dItem::Dump_(Sink& sink) const
{
    InstrDump(sink);
    sink.WriteLittleUint8(tgts.size());
    for (const auto& l : tgts)
        sink.WriteLittleUint32(ToFilePos(l->GetPtr()));
}

void Instruction0dItem::Inspect_(std::ostream& os, unsigned indent) const
{
    InstrInspect(os, indent);
    bool first = true;
    for (const auto& l : tgts)
    {
        if (!first) os << ", ";
        first = false;
        os << PrintLabel(l);
    }
    os << ')';
}

void Instruction0dItem::PostInsert()
{
    for (const auto& l : tgts)
        MaybeCreateUnchecked<InstructionBase>(l->GetPtr());
}

// ------------------------------------------------------------------------

void Instruction1dItem::FixParams::Validate(
    FilePosition rem_size, FilePosition size)
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stsc::Instruction1dItem::FixParams", x)
    VALIDATE(this->size * sizeof(NodeParams) <= rem_size);
    VALIDATE(tgt < size);
#undef VALIDATE
}

void Instruction1dItem::NodeParams::Validate(uint16_t size)
{
#define VALIDATE(x) NEPTOOLS_VALIDATE_FIELD("Stsc::Instruction1dItem::NodeParams", x)
    VALIDATE(left <= size);
    VALIDATE(right <= size);
#undef VALIDATE
}

Instruction1dItem::Instruction1dItem(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}, tgt{EmptyNotNull{}}
{
    AddInfo(&Instruction1dItem::Parse_, ADD_SOURCE(src), this, ctx, src);
}

void Instruction1dItem::Dispose() noexcept
{
    tree.clear();
    InstructionBase::Dispose();
}

void Instruction1dItem::Parse_(Context& ctx, Source& src)
{
    src.CheckRemainingSize(sizeof(FixParams));
    auto fp = src.ReadGen<FixParams>();
    fp.Validate(src.GetRemainingSize(), ctx.GetSize());
    tgt = ctx.GetLabelTo(fp.tgt);

    uint16_t n = fp.size;
    src.CheckRemainingSize(n * sizeof(NodeParams));
    tree.reserve(n);
    for (uint16_t i = 0; i < n; ++i)
    {
        auto nd = src.ReadGen<NodeParams>();
        nd.Validate(n);
        tree.push_back({nd.operation, nd.value, nd.left, nd.right});
    }
}

void Instruction1dItem::Dump_(Sink& sink) const
{
    InstrDump(sink);
    sink.WriteGen(FixParams{tree.size(), ToFilePos(tgt->GetPtr())});
    for (auto& n : tree)
        sink.WriteGen(NodeParams{n.operation, n.value, n.left, n.right});
}

void Instruction1dItem::Inspect_(std::ostream& os, unsigned indent) const
{
    InstrInspect(os, indent) << ", " << PrintLabel(tgt) << ", ";
    InspectNode(os, 0);
    os << ')';
}

void Instruction1dItem::InspectNode(std::ostream& os, size_t i) const
{
    if (i >= tree.size())
    {
        os << "nil";
        return;
    }

    auto& n = tree[i];
    os << '{' << unsigned(n.operation) << ", " << n.value << ", ";
    InspectNode(os, n.left - 1);
    os << ", ";
    InspectNode(os, n.right - 1);
    os << '}';
}

void Instruction1dItem::PostInsert()
{
    MaybeCreateUnchecked<InstructionBase>(tgt->GetPtr());
    MaybeCreateUnchecked<InstructionBase>(&*++Iterator());
}

// ------------------------------------------------------------------------

void Instruction1eItem::FixParams::Validate(FilePosition rem_size)
{
    NEPTOOLS_VALIDATE_FIELD("Stsc::Instruction1eItem::FixParams",
                            size * sizeof(ExpressionParams) <= rem_size);
}

void Instruction1eItem::ExpressionParams::Validate(FilePosition size)
{
    NEPTOOLS_VALIDATE_FIELD("Stsc::Instruction1eItem::ExpressionParams",
                            tgt < size);
}

Instruction1eItem::Instruction1eItem(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}
{
    AddInfo(&Instruction1eItem::Parse_, ADD_SOURCE(src), this, ctx, src);
}

void Instruction1eItem::Dispose() noexcept
{
    expressions.clear();
    InstructionBase::Dispose();
}

void Instruction1eItem::Parse_(Context& ctx, Source& src)
{
    src.CheckRemainingSize(sizeof(FixParams));
    auto fp = src.ReadGen<FixParams>();
    fp.Validate(src.GetRemainingSize());

    field_0 = fp.field_0;
    flag = fp.size & 0x8000;
    auto size = flag ? fp.size & 0x7ff : uint16_t(fp.size);

    expressions.reserve(size);
    for (uint16_t i = 0; i < size; ++i)
    {
        auto exp = src.ReadGen<ExpressionParams>();
        exp.Validate(ctx.GetSize());
        expressions.emplace_back(
                exp.expression, ctx.GetLabelTo(exp.tgt));
    }
}

void Instruction1eItem::Dump_(Sink& sink) const
{
    InstrDump(sink);
    sink.WriteGen(FixParams{field_0, (flag << 15) | expressions.size()});
    for (auto& e : expressions)
        sink.WriteGen(ExpressionParams{
            e.expression, ToFilePos(e.target->GetPtr())});

}

void Instruction1eItem::Inspect_(std::ostream& os, unsigned indent) const
{
    InstrInspect(os, indent) << ", " << field_0 << ", " << flag << ", {";
    bool first = true;
    for (auto& e : expressions)
    {
        if (!first) os << ", ";
        first = false;
        os << '{' << e.expression << ", " << PrintLabel(e.target) << '}';
    }
    os << "})";
}

void Instruction1eItem::PostInsert()
{
    for (const auto& e : expressions)
        MaybeCreate<InstructionBase>(e.target->GetPtr());
    MaybeCreateUnchecked<InstructionBase>(&*++Iterator());
}

}
}

#ifndef NEPTOOLS_WITHOUT_LUA
namespace Neptools::Lua
{

template <bool NoReturn, typename... Args>
struct TypeRegisterTraits<Stsc::SimpleInstruction<NoReturn, Args...>>
{
    using T = Stsc::SimpleInstruction<NoReturn, Args...>;

    template <size_t I>
    static RetNum Get(StateRef vm, T& instr, int idx)
    {
        if constexpr (I == sizeof...(Args))
            lua_pushnil(vm);
        else if (idx == I)
            vm.Push(std::get<I>(instr.args));
        else
            return Get<I+1>(vm, instr, idx);
        return 1;
    }

    template <size_t I>
    static void Set(StateRef vm, T& instr, int idx, Skip val)
    {
        (void) val;
        if constexpr (I == sizeof...(Args))
            luaL_error(vm, "trying to set invalid index");
        else if (idx == I)
            std::get<I>(instr.args) = vm.Check<std::tuple_element_t<
                I, typename T::ArgsT>>(3);
        else
            Set<I+1>(vm, instr, idx, {});
    }

    static void Register(TypeBuilder& bld)
    {
        bld.Inherit<Stsc::InstructionBase>();

        // that tuple constructors can blow up exponentially, disable overload
        // check (tuple constructors can't take source, so it should be ok)
        bld.AddFunction<
            TypeTraits<T>::template Make<
                Context::Key, Context&, uint8_t, Source&>,
            TypeTraits<T>::template Make<
                Context::Key, Context&, uint8_t,
                LuaGetRef<Stsc::TupleTypeMapT<Args>>...>
        >("new");

        bld.AddFunction<&Get<0>>("get");
        bld.AddFunction<&Set<0>>("set");
    }
};

namespace
{
struct NEPTOOLS_NOLUA InstructionItem : StaticClass
{
    constexpr static char TYPE_NAME[] = "neptools.stsc.instruction_item";
};
constexpr char InstructionItem::TYPE_NAME[];

template <typename T> struct InstructionReg;
template <size_t... Idx> struct InstructionReg<std::index_sequence<Idx...>>
{
    static void Register(TypeBuilder& bld)
    {
        auto vm = bld.GetVm();
        ((TypeRegister::Register<Stsc::InstructionItem<Idx>>(vm), lua_rawseti(vm, -3, Idx)), ...);
    }
};

}

template<> struct TypeRegisterTraits<InstructionItem>
    : InstructionReg<std::make_index_sequence<256>> {};

static TypeRegister::StateRegister<InstructionItem> reg;

}

#include "../../container/vector.lua.hpp"
NEPTOOLS_STD_VECTOR_LUAGEN(
    label, Neptools::NotNull<Neptools::LabelPtr>);
NEPTOOLS_STD_VECTOR_LUAGEN(
    stsc_instruction1d_item_node, Neptools::Stsc::Instruction1dItem::Node);
NEPTOOLS_STD_VECTOR_LUAGEN(
    stsc_instruction1e_item_expression,
    Neptools::Stsc::Instruction1eItem::Expression);
#include "instruction.binding.hpp"

#endif
