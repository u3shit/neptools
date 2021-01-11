#include "instruction.hpp"

#include "../cstring_item.hpp"
#include "../raw_item.hpp"
#include "../../sink.hpp"

#include <libshit/lua/static_class.hpp>
#include <libshit/lua/user_type.hpp>
#include <iomanip>

#include <brigand/sequences/list.hpp>
#include <brigand/algorithms/transform.hpp>
#include <brigand/sequences/make_sequence.hpp>
#include <brigand/algorithms/flatten.hpp>

namespace Neptools::Stsc
{

  // helpers for generic CreateAndInsert
  namespace
  {
    using CreateType = Libshit::NotNull<Libshit::SmartPtr<InstructionBase>>
      (*)(Context&, const Source&);

    template <Flavor F, uint8_t I>
    Libshit::NotNull<Libshit::SmartPtr<InstructionBase>>
    CreateAdapt(Context& ctx, const Source& src)
    { return ctx.Create<InstructionItem<F, I>>(I, src); }


#define NEPTOOLS_GEN(x,y) , Flavor::x
    using Flavors = brigand::integral_list<
      Flavor NEPTOOLS_GEN_STSC_FLAVOR(NEPTOOLS_GEN,)>;
#undef NEPTOOLS_GEN

    template <typename F>
    using MakeMap = brigand::transform<
      brigand::make_sequence<brigand::uint32_t<0>, 256>,
      brigand::bind<brigand::pair, F, brigand::_1>>;
    using AllOpcodes = brigand::flatten<
      brigand::transform<Flavors, brigand::bind<MakeMap, brigand::_1>>>;

    template <typename List> struct CreateMapImpl;
    template <typename... X>
    struct CreateMapImpl<brigand::list<X...>>
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
      static inline const constexpr CreateType
      MAP[brigand::size<Flavors>::value][256] =
      { CreateAdapt<X::first_type::value, X::second_type::value>... };
#pragma GCC diagnostic pop
    };

    using CreateMap = CreateMapImpl<AllOpcodes>;
  }

  // base
  InstructionBase& InstructionBase::CreateAndInsert(ItemPointer ptr, Flavor f)
  {
    auto x = RawItem::GetSource(ptr, -1);
    x.src.CheckSize(1);
    uint8_t opcode = x.src.ReadLittleUint8();
    auto ctx = x.ritem.GetContext();
    auto& ret = x.ritem.Split(
      ptr.offset, CreateMap::MAP[static_cast<size_t>(f)][opcode](*ctx, x.src));

    ret.PostInsert(f);
    return ret;
  }

  void InstructionBase::InstrDump(Sink& sink) const
  {
    sink.WriteLittleUint8(opcode);
  }

  std::ostream& InstructionBase::InstrInspect(
    std::ostream& os, unsigned indent) const
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
      static void PostInsert(T, Flavor) {}
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
      static void PostInsert(float, Flavor) {}
    };

    template<> struct Traits<void*>
    {
      using RawType = boost::endian::little_uint32_t;
      static constexpr const size_t SIZE = 4;

      static void Validate(uint32_t r, FilePosition size)
      { LIBSHIT_VALIDATE_FIELD("Stsc::Instruction", r <= size); }

      static LabelPtr Parse(uint32_t r, Context& ctx)
      { return r ? ctx.GetLabelTo(r) : LabelPtr{}; }

      static RawType Dump(const LabelPtr& l)
      { return l ? ToFilePos(l->GetPtr()) : 0; }

      static void Inspect(std::ostream& os, const LabelPtr& l)
      { os << PrintLabel(l); }

      static void PostInsert(const LabelPtr&, Flavor) {}
    };

    template<> struct Traits<std::string> : public Traits<void*>
    {
      static LabelPtr Parse(uint32_t r, Context& ctx)
      {
        if (!r) return {};
        if (auto ptr = ctx.GetPointer(r); ptr.Maybe<RawItem>())
        {
          auto x = RawItem::GetSource(ptr, -1);
          return ctx.GetLabelTo(
            r, CStringItem::GetLabelName(x.src.PreadCString(0)));
        }
        else
          return ctx.GetLabelTo(r);
      }

      static void PostInsert(const LabelPtr& lbl, Flavor)
      { if (lbl) MaybeCreate<CStringItem>(lbl->GetPtr()); }
    };

    template<> struct Traits<Code*> : public Traits<void*>
    {
      static void PostInsert(const LabelPtr& lbl, Flavor f)
      { if (lbl) MaybeCreateUnchecked<InstructionBase>(lbl->GetPtr(), f); }
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
      static void PostInsert(const Tuple& tuple, Flavor f)
      {
        (void) f; // shut up, retarded gcc
        FORALL(Traits<T>::PostInsert(std::get<I>(tuple), f));
      }

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
    ADD_SOURCE(Parse_(ctx, src), src);
  }

  template <bool NoReturn, typename... Args>
  void SimpleInstruction<NoReturn, Args...>::Parse_(Context& ctx, Source& src)
  {
    src.CheckSize(SIZE);
    using Tuple = PODTuple<typename Traits<Args>::RawType...>;
    static_assert(std::is_pod_v<Tuple>);
    static_assert(Libshit::EmptySizeof<Tuple> == Operations<Args...>::Size());

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
  void SimpleInstruction<NoReturn, Args...>::PostInsert(Flavor f)
  {
    Operations<Args...>::PostInsert(args, f);
    if (!NoReturn) MaybeCreateUnchecked<InstructionBase>(&*++Iterator(), f);
  }

  // ------------------------------------------------------------------------
  // specific instruction implementations
  InstructionRndJumpItem::InstructionRndJumpItem(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}
  {
    ADD_SOURCE(Parse_(ctx, src), src);
  }

  void InstructionRndJumpItem::Parse_(Context& ctx, Source& src)
  {
    src.CheckRemainingSize(1);
    uint8_t n = src.ReadLittleUint8();
    src.CheckRemainingSize(4*n);

    tgts.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
      uint32_t t = src.ReadLittleUint32();
      LIBSHIT_VALIDATE_FIELD(
        "Stsc::InstructionRndJumpItem", t < ctx.GetSize());
      tgts.push_back(ctx.GetLabelTo(t));
    }
  }

  void InstructionRndJumpItem::Dump_(Sink& sink) const
  {
    InstrDump(sink);
    sink.WriteLittleUint8(tgts.size());
    for (const auto& l : tgts)
      sink.WriteLittleUint32(ToFilePos(l->GetPtr()));
  }

  void InstructionRndJumpItem::Inspect_(std::ostream& os, unsigned indent) const
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

  void InstructionRndJumpItem::PostInsert(Flavor f)
  {
    for (const auto& l : tgts)
      MaybeCreateUnchecked<InstructionBase>(l->GetPtr(), f);
    MaybeCreateUnchecked<InstructionBase>(&*++Iterator(), f);
  }

  // ------------------------------------------------------------------------

  void InstructionJumpIfItem::FixParams::Validate(
    FilePosition rem_size, FilePosition size)
  {
#define VALIDATE(x) \
    LIBSHIT_VALIDATE_FIELD("Stsc::InstructionJumpIfItem::FixParams", x)
    VALIDATE(this->size * sizeof(NodeParams) <= rem_size);
    VALIDATE(tgt < size);
#undef VALIDATE
  }

  void InstructionJumpIfItem::NodeParams::Validate(uint16_t size)
  {
#define VALIDATE(x) \
    LIBSHIT_VALIDATE_FIELD("Stsc::InstructionJumpIfItem::NodeParams", x)
    VALIDATE(left <= size);
    VALIDATE(right <= size);
#undef VALIDATE
  }

  InstructionJumpIfItem::InstructionJumpIfItem(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}, tgt{Libshit::EmptyNotNull{}}
  {
    ADD_SOURCE(Parse_(ctx, src), src);
  }

#if LIBSHIT_WITH_LUA
  static size_t ParseTree(
    Libshit::Lua::StateRef vm, std::vector<InstructionJumpIfItem::Node>& tree,
    Libshit::Lua::Any lua_tree, int type)
  {
    if (type == LUA_TNIL) return 0;

    lua_checkstack(vm, 5);

    auto i = tree.size();
    tree.emplace_back();

    lua_rawgeti(vm, lua_tree, 1); // +1
    tree[i].operation = vm.Check<uint8_t>(-1);
    lua_rawgeti(vm, lua_tree, 2); // +2
    tree[i].value = vm.Check<uint32_t>(-1);
    lua_pop(vm, 2); // 0

    type = lua_rawgeti(vm, lua_tree, 3); // +1
    tree[i].left = ParseTree(vm, tree, lua_gettop(vm), type);
    lua_pop(vm, 1); // 0

    type = lua_rawgeti(vm, lua_tree, 4); // +1
    tree[i].right = ParseTree(vm, tree, lua_gettop(vm), type);
    lua_pop(vm, 1); // 0
    return i+1;
  }

  InstructionJumpIfItem::InstructionJumpIfItem(
    Key k, Context& ctx, Libshit::Lua::StateRef vm, uint8_t opcode,
    Libshit::NotNull<LabelPtr> tgt, Libshit::Lua::RawTable lua_tree)
    : InstructionBase{k, ctx, opcode}, tgt{tgt}
  { ParseTree(vm, tree, lua_tree, LUA_TTABLE); }
#endif

  void InstructionJumpIfItem::Dispose() noexcept
  {
    tree.clear();
    InstructionBase::Dispose();
  }

  void InstructionJumpIfItem::Parse_(Context& ctx, Source& src)
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

  void InstructionJumpIfItem::Dump_(Sink& sink) const
  {
    InstrDump(sink);
    sink.WriteGen(FixParams{tree.size(), ToFilePos(tgt->GetPtr())});
    for (auto& n : tree)
      sink.WriteGen(NodeParams{n.operation, n.value, n.left, n.right});
  }

  void InstructionJumpIfItem::Inspect_(std::ostream& os, unsigned indent) const
  {
    InstrInspect(os, indent) << ", " << PrintLabel(tgt) << ", ";
    InspectNode(os, 0);
    os << ')';
  }

  void InstructionJumpIfItem::InspectNode(std::ostream& os, size_t i) const
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

  void InstructionJumpIfItem::PostInsert(Flavor f)
  {
    MaybeCreateUnchecked<InstructionBase>(tgt->GetPtr(), f);
    MaybeCreateUnchecked<InstructionBase>(&*++Iterator(), f);
  }

  // ------------------------------------------------------------------------

  void InstructionJumpSwitchItemNoire::FixParams::Validate(FilePosition rem_size)
  {
    LIBSHIT_VALIDATE_FIELD("Stsc::InstructionJumpSwitchItemNoire::FixParams",
                           size * sizeof(ExpressionParams) <= rem_size);
  }

  void InstructionJumpSwitchItemNoire::ExpressionParams::Validate(
    FilePosition size)
  {
    LIBSHIT_VALIDATE_FIELD(
      "Stsc::InstructionJumpSwitchItemNoire::ExpressionParams", tgt < size);
  }

  InstructionJumpSwitchItemNoire::InstructionJumpSwitchItemNoire(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionBase{k, ctx, opcode}
  {
    ADD_SOURCE(Parse_(ctx, src), src);
  }

  InstructionJumpSwitchItemPotbb::InstructionJumpSwitchItemPotbb(
    Key k, Context& ctx, uint8_t opcode, Source src)
    : InstructionJumpSwitchItemNoire{k, ctx, opcode}
  {
    ADD_SOURCE(Parse_(ctx, src), src);
  }

  void InstructionJumpSwitchItemNoire::Dispose() noexcept
  {
    expressions.clear();
    InstructionBase::Dispose();
  }

  void InstructionJumpSwitchItemNoire::Parse_(Context& ctx, Source& src)
  {
    src.CheckRemainingSize(sizeof(FixParams));
    auto fp = src.ReadGen<FixParams>();
    fp.Validate(src.GetRemainingSize());

    expected_val = fp.expected_val;
    last_is_default = fp.size & 0x8000;
    auto size = last_is_default ? fp.size & 0x7ff : uint16_t(fp.size);

    expressions.reserve(size);
    for (uint16_t i = 0; i < size; ++i)
    {
      auto exp = src.ReadGen<ExpressionParams>();
      exp.Validate(ctx.GetSize());
      expressions.emplace_back(
        exp.expression, ctx.GetLabelTo(exp.tgt));
    }
  }

  void InstructionJumpSwitchItemPotbb::Parse_(Context& ctx, Source& src)
  {
    InstructionJumpSwitchItemNoire::Parse_(ctx, src);
    src.CheckRemainingSize(1);
    trailing_byte = src.ReadLittleUint8();
  }

  void InstructionJumpSwitchItemNoire::Dump_(Sink& sink) const
  {
    InstrDump(sink);
    sink.WriteGen(FixParams{expected_val,
                            (last_is_default << 15) | expressions.size()});
    for (auto& e : expressions)
      sink.WriteGen(ExpressionParams{
          e.expression, ToFilePos(e.target->GetPtr())});
  }

  void InstructionJumpSwitchItemPotbb::Dump_(Sink& sink) const
  {
    InstructionJumpSwitchItemNoire::Dump_(sink);
    sink.WriteLittleUint8(trailing_byte);
  }

  void InstructionJumpSwitchItemNoire::InspectBase(
    std::ostream& os, unsigned indent) const
  {
    InstrInspect(os, indent) << ", " << expected_val << ", "
                             << last_is_default << ", {";
    bool first = true;
    for (auto& e : expressions)
    {
      if (!first) os << ", ";
      first = false;
      os << '{' << e.expression << ", " << PrintLabel(e.target) << '}';
    }
    os << '}';
  }

  void InstructionJumpSwitchItemNoire::Inspect_(
    std::ostream& os, unsigned indent) const
  {
    InspectBase(os, indent);
    os << ')';
  }

  void InstructionJumpSwitchItemPotbb::Inspect_(
    std::ostream& os, unsigned indent) const
  {
    InspectBase(os, indent);
    os << ", " << int(trailing_byte) << ')';
  }

  void InstructionJumpSwitchItemNoire::PostInsert(Flavor f)
  {
    for (const auto& e : expressions)
      MaybeCreate<InstructionBase>(e.target->GetPtr(), f);
    if (!last_is_default)
      MaybeCreateUnchecked<InstructionBase>(&*++Iterator(), f);
  }

}

#if LIBSHIT_WITH_LUA
namespace Libshit::Lua
{

  template <bool NoReturn, typename... Args>
  struct TypeRegisterTraits<Neptools::Stsc::SimpleInstruction<NoReturn, Args...>>
  {
    using T = Neptools::Stsc::SimpleInstruction<NoReturn, Args...>;

    template <size_t I>
    static RetNum Get0(StateRef vm, T& instr, int idx)
    {
      if constexpr (I == sizeof...(Args))
        lua_pushnil(vm);
      else if (idx == I)
        vm.Push(std::get<I>(instr.args));
      else
        return Get0<I+1>(vm, instr, idx);
      return 1;
    }
    static void Get1(const T&, Libshit::Lua::VarArg) noexcept {}


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
      bld.Inherit<T, Neptools::Stsc::InstructionBase>();

      // that tuple constructors can blow up exponentially, disable overload
      // check (tuple constructors can't take source, so it should be ok)
      bld.AddFunction<
        TypeTraits<T>::template Make<
          Neptools::Context::Key, Neptools::Context&, uint8_t, Neptools::Source&>,
        TypeTraits<T>::template Make<
          Neptools::Context::Key, Neptools::Context&, uint8_t,
          LuaGetRef<Neptools::Stsc::TupleTypeMapT<Args>>...>
      >("new");

      bld.AddFunction<&Get0<0>, &Get1>("get");
      bld.AddFunction<&Set<0>>("set");
    }
  };

  namespace
  {
    struct LIBSHIT_NOLUA InstructionItem : StaticClass
    {
      constexpr static char TYPE_NAME[] = "neptools.stsc.instruction_item";
    };
    constexpr char InstructionItem::TYPE_NAME[];

    template <typename T> struct InstructionReg;
    template <typename... X> struct InstructionReg<brigand::list<X...>>
    {
      static void GetTab(Lua::StateRef vm, int i, int& prev)
      {
        if (i == prev) return;
        if (prev != -1) lua_pop(vm, 1);
        prev = i;
        lua_createtable(vm, 255, 0);
        lua_pushvalue(vm, -1);
        lua_rawseti(vm, -4, i);
      }

      static void Register(TypeBuilder& bld)
      {
        static_assert(sizeof...(X) > 0);
        auto vm = bld.GetVm();
        int prev = -1;
        ((GetTab(vm, static_cast<int>(X::first_type::value), prev),
          TypeRegister::Register<Neptools::Stsc::InstructionItem<
          X::first_type::value, X::second_type::value>>(vm),
          lua_rawseti(vm, -2, X::second_type::value)), ...);
        lua_pop(vm, 1);
      }
    };

  }

  template<> struct TypeRegisterTraits<InstructionItem>
    : InstructionReg<Neptools::Stsc::AllOpcodes> {};

  static TypeRegister::StateRegister<InstructionItem> reg;

}

#include <libshit/container/vector.lua.hpp>
LIBSHIT_STD_VECTOR_LUAGEN(
  label, Libshit::NotNull<Neptools::LabelPtr>);
LIBSHIT_STD_VECTOR_LUAGEN(
  stsc_instruction_jump_if_item_node,
  Neptools::Stsc::InstructionJumpIfItem::Node);
LIBSHIT_STD_VECTOR_LUAGEN(
  stsc_instruction_jump_switch_item_noire_expression,
  Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression);
#include "instruction.binding.hpp"

#endif
