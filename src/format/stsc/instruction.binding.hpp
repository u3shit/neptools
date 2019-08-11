// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stsc::InstructionBase::TYPE_NAME[] = "neptools.stsc.instruction_base";

const char ::Neptools::Stsc::InstructionRndJumpItem::TYPE_NAME[] = "neptools.stsc.instruction_rnd_jump_item";

const char ::Neptools::Stsc::UnimplementedInstructionItem::TYPE_NAME[] = "neptools.stsc.unimplemented_instruction_item";

const char ::Neptools::Stsc::InstructionJumpIfItem::TYPE_NAME[] = "neptools.stsc.instruction_jump_if_item";

const char ::Neptools::Stsc::InstructionJumpIfItem::Node::TYPE_NAME[] = "neptools.stsc.instruction_jump_if_item.node";

const char ::Neptools::Stsc::InstructionJumpSwitchItemNoire::TYPE_NAME[] = "neptools.stsc.instruction_jump_switch_item_noire";

const char ::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression::TYPE_NAME[] = "neptools.stsc.instruction_jump_switch_item_noire.expression";

const char ::Neptools::Stsc::InstructionJumpSwitchItemPotbb::TYPE_NAME[] = "neptools.stsc.instruction_jump_switch_item_potbb";

namespace Libshit::Lua
{

  // class neptools.stsc.instruction_base
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionBase>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionBase, ::Neptools::Item>();

    bld.AddFunction<
      static_cast<::Neptools::Stsc::InstructionBase & (*)(::Neptools::ItemPointer, ::Neptools::Stsc::Flavor)>(::Neptools::Stsc::InstructionBase::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionBase, const ::uint8_t, &::Neptools::Stsc::InstructionBase::opcode>
    >("get_opcode");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionBase> reg_neptools_stsc_instruction_base;

  // class neptools.stsc.instruction_rnd_jump_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionRndJumpItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionRndJumpItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionRndJumpItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::InstructionRndJumpItem, std::vector<::Libshit::NotNull<::Neptools::LabelPtr> >, &::Neptools::Stsc::InstructionRndJumpItem::tgts>
    >("get_tgts");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionRndJumpItem> reg_neptools_stsc_instruction_rnd_jump_item;

  // class neptools.stsc.unimplemented_instruction_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::UnimplementedInstructionItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::UnimplementedInstructionItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::UnimplementedInstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<const ::Neptools::Source &>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::UnimplementedInstructionItem> reg_neptools_stsc_unimplemented_instruction_item;

  // class neptools.stsc.instruction_jump_if_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionJumpIfItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionJumpIfItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpIfItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpIfItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<std::vector<::Neptools::Stsc::InstructionJumpIfItem::Node>>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpIfItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Libshit::Lua::StateRef>, LuaGetRef<::uint8_t>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Libshit::Lua::RawTable>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpIfItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::InstructionJumpIfItem::tgt>
    >("get_tgt");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::InstructionJumpIfItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::InstructionJumpIfItem::tgt>
    >("set_tgt");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::InstructionJumpIfItem, std::vector<::Neptools::Stsc::InstructionJumpIfItem::Node>, &::Neptools::Stsc::InstructionJumpIfItem::tree>
    >("get_tree");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionJumpIfItem> reg_neptools_stsc_instruction_jump_if_item;

  // class neptools.stsc.instruction_jump_if_item.node
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionJumpIfItem::Node>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpIfItem::Node, ::uint8_t, &::Neptools::Stsc::InstructionJumpIfItem::Node::operation>
    >("get_operation");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpIfItem::Node, ::uint32_t, &::Neptools::Stsc::InstructionJumpIfItem::Node::value>
    >("get_value");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpIfItem::Node, ::size_t, &::Neptools::Stsc::InstructionJumpIfItem::Node::left>
    >("get_left");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpIfItem::Node, ::size_t, &::Neptools::Stsc::InstructionJumpIfItem::Node::right>
    >("get_right");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpIfItem::Node>::Make<>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpIfItem::Node>::Make<LuaGetRef<::uint8_t>, LuaGetRef<::uint32_t>, LuaGetRef<::size_t>, LuaGetRef<::size_t>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionJumpIfItem::Node> reg_neptools_stsc_instruction_jump_if_item_node;

  // class neptools.stsc.instruction_jump_switch_item_noire
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionJumpSwitchItemNoire>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionJumpSwitchItemNoire, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpSwitchItemNoire>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpSwitchItemNoire>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::uint32_t>, LuaGetRef<bool>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression> >>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire, ::uint32_t, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::expected_val>
    >("get_expected_val");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire, ::uint32_t, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::expected_val>
    >("set_expected_val");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire, bool, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::last_is_default>
    >("get_last_is_default");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire, bool, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::last_is_default>
    >("set_last_is_default");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire, std::vector<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression>, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::expressions>
    >("get_expressions");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionJumpSwitchItemNoire> reg_neptools_stsc_instruction_jump_switch_item_noire;

  // class neptools.stsc.instruction_jump_switch_item_noire.expression
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression, ::uint32_t, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression::expression>
    >("get_expression");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression::target>
    >("get_target");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression>::Make<LuaGetRef<::uint32_t>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression> reg_neptools_stsc_instruction_jump_switch_item_noire_expression;

  // class neptools.stsc.instruction_jump_switch_item_potbb
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionJumpSwitchItemPotbb>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionJumpSwitchItemPotbb, ::Neptools::Stsc::InstructionJumpSwitchItemNoire>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpSwitchItemPotbb>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>,
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::InstructionJumpSwitchItemPotbb>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::uint32_t>, LuaGetRef<bool>, LuaGetRef<Libshit::AT<std::vector<::Neptools::Stsc::InstructionJumpSwitchItemNoire::Expression> >>, LuaGetRef<::uint8_t>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionJumpSwitchItemPotbb, ::uint8_t, &::Neptools::Stsc::InstructionJumpSwitchItemPotbb::trailing_byte>
    >("get_trailing_byte");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::InstructionJumpSwitchItemPotbb, ::uint8_t, &::Neptools::Stsc::InstructionJumpSwitchItemPotbb::trailing_byte>
    >("set_trailing_byte");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionJumpSwitchItemPotbb> reg_neptools_stsc_instruction_jump_switch_item_potbb;

}
#endif
