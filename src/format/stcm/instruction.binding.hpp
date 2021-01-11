// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stcm::InstructionItem::TYPE_NAME[] = "neptools.stcm.instruction_item";

const char ::Neptools::Stcm::InstructionItem::Param48::TYPE_NAME[] = "neptools.stcm.instruction_item.param48";
const char ::Libshit::Lua::TypeName<::Neptools::Stcm::InstructionItem::Param48::Type>::TYPE_NAME[] =
  "neptools.stcm.instruction_item.param48.type";

const char ::Neptools::Stcm::InstructionItem::Param::TYPE_NAME[] = "neptools.stcm.instruction_item.param";

const char ::Neptools::Stcm::InstructionItem::Param::MemOffset::TYPE_NAME[] = "neptools.stcm.instruction_item.param.mem_offset";

const char ::Neptools::Stcm::InstructionItem::Param::Indirect::TYPE_NAME[] = "neptools.stcm.instruction_item.param.indirect";
const char ::Libshit::Lua::TypeName<::Neptools::Stcm::InstructionItem::Param::Type>::TYPE_NAME[] =
  "neptools.stcm.instruction_item.param.type";

namespace Libshit::Lua
{

  // class neptools.stcm.instruction_item
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stcm::InstructionItem, ::Neptools::ItemWithChildren>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stcm::InstructionItem::Param> >>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stcm::InstructionItem::Param> >>>
    >("new");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem & (*)(::Neptools::ItemPointer)>(::Neptools::Stcm::InstructionItem::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      static_cast<bool (::Neptools::Stcm::InstructionItem::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::IsCall)
    >("is_call");
    bld.AddFunction<
      static_cast<::uint32_t (::Neptools::Stcm::InstructionItem::*)() const>(&::Neptools::Stcm::InstructionItem::GetOpcode)
    >("get_opcode");
    bld.AddFunction<
      static_cast<void (::Neptools::Stcm::InstructionItem::*)(::uint32_t) noexcept>(&::Neptools::Stcm::InstructionItem::SetOpcode)
    >("set_opcode");
    bld.AddFunction<
      static_cast<::Libshit::NotNull<::Neptools::LabelPtr> (::Neptools::Stcm::InstructionItem::*)() const>(&::Neptools::Stcm::InstructionItem::GetTarget)
    >("get_target");
    bld.AddFunction<
      static_cast<void (::Neptools::Stcm::InstructionItem::*)(::Libshit::NotNull<::Neptools::LabelPtr>) noexcept>(&::Neptools::Stcm::InstructionItem::SetTarget)
    >("set_target");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stcm::InstructionItem, std::vector<::Neptools::Stcm::InstructionItem::Param>, &::Neptools::Stcm::InstructionItem::params>
    >("get_params");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem> reg_neptools_stcm_instruction_item;

  // class neptools.stcm.instruction_item.param48
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param48>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param48>::Make<LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>>
    >("new");
    bld.AddFunction<
      static_cast<::uint32_t (::Neptools::Stcm::InstructionItem::Param48::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param48::Dump)
    >("dump");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48::Type (::Neptools::Stcm::InstructionItem::Param48::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param48::GetType)
    >("get_type");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>)
    >("get_mem_offset");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>)
    >("get_immediate");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>)
    >("get_indirect");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>)
    >("get_read_stack");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>)
    >("get_read_4ac");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>)
    >("new_mem_offset");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>)
    >("new_immediate");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>)
    >("new_indirect");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>)
    >("new_read_stack");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>)
    >("new_read_4ac");
bld.TaggedNew();
  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param48> reg_neptools_stcm_instruction_item_param48;

  // class neptools.stcm.instruction_item.param48.type
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param48::Type>::Register(TypeBuilder& bld)
  {

    bld.Add("MEM_OFFSET", ::Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET);
    bld.Add("IMMEDIATE", ::Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE);
    bld.Add("INDIRECT", ::Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT);
    bld.Add("READ_STACK", ::Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK);
    bld.Add("READ_4AC", ::Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC);

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param48::Type> reg_neptools_stcm_instruction_item_param48_type;

  // class neptools.stcm.instruction_item.param
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      static_cast<void (::Neptools::Stcm::InstructionItem::Param::*)(::Neptools::Sink &) const>(&::Neptools::Stcm::InstructionItem::Param::Dump)
    >("dump");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param::Type (::Neptools::Stcm::InstructionItem::Param::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param::GetType)
    >("get_type");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>)
    >("get_mem_offset");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>)
    >("get_indirect");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>)
    >("get_read_stack");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>)
    >("get_read_4ac");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>)
    >("get_instr_ptr0");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>)
    >("get_instr_ptr1");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>)
    >("get_coll_link");
    bld.AddFunction<
      static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::EXPANSION>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::EXPANSION>)
    >("get_expansion");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>),
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(::Libshit::NotNull<::Neptools::LabelPtr>, Libshit::AT<::Neptools::Stcm::InstructionItem::Param48>, Libshit::AT<::Neptools::Stcm::InstructionItem::Param48>)>(::Neptools::Stcm::InstructionItem::Param::NewMemOffset)
    >("new_mem_offset");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INDIRECT), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>),
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(::uint32_t, Libshit::AT<::Neptools::Stcm::InstructionItem::Param48>)>(::Neptools::Stcm::InstructionItem::Param::NewIndirect)
    >("new_indirect");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::READ_STACK), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>)
    >("new_read_stack");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::READ_4AC), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>)
    >("new_read_4ac");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>)
    >("new_instr_ptr0");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>)
    >("new_instr_ptr1");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>)
    >("new_coll_link");
    bld.AddFunction<
      static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::EXPANSION), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::EXPANSION>)
    >("new_expansion");
bld.TaggedNew();
  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param> reg_neptools_stcm_instruction_item_param;

  // class neptools.stcm.instruction_item.param.mem_offset
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param::MemOffset>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::InstructionItem::Param::MemOffset::target>
    >("get_target");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::MemOffset::param_4>
    >("get_param_4");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::MemOffset::param_8>
    >("get_param_8");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param::MemOffset>::Make<LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::MemOffset> reg_neptools_stcm_instruction_item_param_mem_offset;

  // class neptools.stcm.instruction_item.param.indirect
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param::Indirect>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::Indirect, ::uint32_t, &::Neptools::Stcm::InstructionItem::Param::Indirect::param_0>
    >("get_param_0");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::Indirect, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::Indirect::param_8>
    >("get_param_8");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param::Indirect>::Make<LuaGetRef<::uint32_t>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::Indirect> reg_neptools_stcm_instruction_item_param_indirect;

  // class neptools.stcm.instruction_item.param.type
  template<>
  void TypeRegisterTraits<::Neptools::Stcm::InstructionItem::Param::Type>::Register(TypeBuilder& bld)
  {

    bld.Add("MEM_OFFSET", ::Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET);
    bld.Add("INDIRECT", ::Neptools::Stcm::InstructionItem::Param::Type::INDIRECT);
    bld.Add("READ_STACK", ::Neptools::Stcm::InstructionItem::Param::Type::READ_STACK);
    bld.Add("READ_4AC", ::Neptools::Stcm::InstructionItem::Param::Type::READ_4AC);
    bld.Add("INSTR_PTR0", ::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0);
    bld.Add("INSTR_PTR1", ::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1);
    bld.Add("COLL_LINK", ::Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK);
    bld.Add("EXPANSION", ::Neptools::Stcm::InstructionItem::Param::Type::EXPANSION);

  }
  static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::Type> reg_neptools_stcm_instruction_item_param_type;

}
#endif
