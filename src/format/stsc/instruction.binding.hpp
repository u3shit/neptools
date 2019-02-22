// Auto generated code, do not edit. See gen_binding in project root.
#if LIBSHIT_WITH_LUA
#include <libshit/lua/user_type.hpp>


const char ::Neptools::Stsc::InstructionBase::TYPE_NAME[] = "neptools.stsc.instruction_base";

const char ::Neptools::Stsc::Instruction0dItem::TYPE_NAME[] = "neptools.stsc.instruction0d_item";

const char ::Neptools::Stsc::UnimplementedInstructionItem::TYPE_NAME[] = "neptools.stsc.unimplemented_instruction_item";

const char ::Neptools::Stsc::Instruction1dItem::TYPE_NAME[] = "neptools.stsc.instruction1d_item";

const char ::Neptools::Stsc::Instruction1dItem::Node::TYPE_NAME[] = "neptools.stsc.instruction1d_item.node";

const char ::Neptools::Stsc::Instruction1eItem::TYPE_NAME[] = "neptools.stsc.instruction1e_item";

const char ::Neptools::Stsc::Instruction1eItem::Expression::TYPE_NAME[] = "neptools.stsc.instruction1e_item.expression";

namespace Libshit::Lua
{

  // class neptools.stsc.instruction_base
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::InstructionBase>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::InstructionBase, ::Neptools::Item>();

    bld.AddFunction<
      static_cast<::Neptools::Stsc::InstructionBase & (*)(::Neptools::ItemPointer)>(::Neptools::Stsc::InstructionBase::CreateAndInsert)
    >("create_and_insert");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::InstructionBase, const ::uint8_t, &::Neptools::Stsc::InstructionBase::opcode>
    >("get_opcode");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::InstructionBase> reg_neptools_stsc_instruction_base;

  // class neptools.stsc.instruction0d_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Instruction0dItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::Instruction0dItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::Instruction0dItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::Instruction0dItem, std::vector<::Libshit::NotNull<::Neptools::LabelPtr> >, &::Neptools::Stsc::Instruction0dItem::tgts>
    >("get_tgts");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Instruction0dItem> reg_neptools_stsc_instruction0d_item;

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

  // class neptools.stsc.instruction1d_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Instruction1dItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::Instruction1dItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::Instruction1dItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1dItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::Instruction1dItem::tgt>
    >("get_tgt");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::Instruction1dItem, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::Instruction1dItem::tgt>
    >("set_tgt");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::Instruction1dItem, std::vector<::Neptools::Stsc::Instruction1dItem::Node>, &::Neptools::Stsc::Instruction1dItem::tree>
    >("get_tree");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Instruction1dItem> reg_neptools_stsc_instruction1d_item;

  // class neptools.stsc.instruction1d_item.node
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Instruction1dItem::Node>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1dItem::Node, ::uint8_t, &::Neptools::Stsc::Instruction1dItem::Node::operation>
    >("get_operation");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1dItem::Node, ::uint32_t, &::Neptools::Stsc::Instruction1dItem::Node::value>
    >("get_value");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1dItem::Node, ::size_t, &::Neptools::Stsc::Instruction1dItem::Node::left>
    >("get_left");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1dItem::Node, ::size_t, &::Neptools::Stsc::Instruction1dItem::Node::right>
    >("get_right");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::Instruction1dItem::Node>::Make<LuaGetRef<::uint8_t>, LuaGetRef<::uint32_t>, LuaGetRef<::size_t>, LuaGetRef<::size_t>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Instruction1dItem::Node> reg_neptools_stsc_instruction1d_item_node;

  // class neptools.stsc.instruction1e_item
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Instruction1eItem>::Register(TypeBuilder& bld)
  {
    bld.Inherit<::Neptools::Stsc::Instruction1eItem, ::Neptools::Stsc::InstructionBase>();

    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::Instruction1eItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint8_t>, LuaGetRef<::Neptools::Source>>
    >("new");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1eItem, ::uint32_t, &::Neptools::Stsc::Instruction1eItem::field_0>
    >("get_field_0");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::Instruction1eItem, ::uint32_t, &::Neptools::Stsc::Instruction1eItem::field_0>
    >("set_field_0");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1eItem, bool, &::Neptools::Stsc::Instruction1eItem::flag>
    >("get_flag");
    bld.AddFunction<
      &::Libshit::Lua::SetMember<::Neptools::Stsc::Instruction1eItem, bool, &::Neptools::Stsc::Instruction1eItem::flag>
    >("set_flag");
    bld.AddFunction<
      &::Libshit::Lua::GetSmartOwnedMember<::Neptools::Stsc::Instruction1eItem, std::vector<::Neptools::Stsc::Instruction1eItem::Expression>, &::Neptools::Stsc::Instruction1eItem::expressions>
    >("get_expressions");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Instruction1eItem> reg_neptools_stsc_instruction1e_item;

  // class neptools.stsc.instruction1e_item.expression
  template<>
  void TypeRegisterTraits<::Neptools::Stsc::Instruction1eItem::Expression>::Register(TypeBuilder& bld)
  {

    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1eItem::Expression, ::uint32_t, &::Neptools::Stsc::Instruction1eItem::Expression::expression>
    >("get_expression");
    bld.AddFunction<
      &::Libshit::Lua::GetMember<::Neptools::Stsc::Instruction1eItem::Expression, ::Libshit::NotNull<::Neptools::LabelPtr>, &::Neptools::Stsc::Instruction1eItem::Expression::target>
    >("get_target");
    bld.AddFunction<
      &::Libshit::Lua::TypeTraits<::Neptools::Stsc::Instruction1eItem::Expression>::Make<LuaGetRef<::uint32_t>, LuaGetRef<::Libshit::NotNull<::Neptools::LabelPtr>>>
    >("new");

  }
  static TypeRegister::StateRegister<::Neptools::Stsc::Instruction1eItem::Expression> reg_neptools_stsc_instruction1e_item_expression;

}
#endif
