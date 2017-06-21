// Auto generated code, do not edit. See gen_binding in project root.
#ifndef NEPTOOLS_WITHOUT_LUA
#include "lua/user_type.hpp"

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem>(TypeBuilder& bld)
{
    bld.Inherit<::Neptools::Stcm::InstructionItem, ::Neptools::ItemWithChildren>();

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>, LuaGetRef<::Neptools::Source>>,
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::InstructionItem>::Make<LuaGetRef<::Neptools::Item::Key>, LuaGetRef<::Neptools::Context &>>
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
        static_cast<::Neptools::NotNull<::Neptools::LabelPtr> (::Neptools::Stcm::InstructionItem::*)() const>(&::Neptools::Stcm::InstructionItem::GetTarget)
    >("get_target");
    bld.AddFunction<
        static_cast<void (::Neptools::Stcm::InstructionItem::*)(::Neptools::NotNull<::Neptools::LabelPtr>) noexcept>(&::Neptools::Stcm::InstructionItem::SetTarget)
    >("set_target");
    bld.AddFunction<
        &::Neptools::Lua::GetSmartOwnedMember<::Neptools::Stcm::InstructionItem, std::vector<::Neptools::Stcm::InstructionItem::Param>, &::Neptools::Stcm::InstructionItem::params>
    >("get_params");

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem> reg_neptools_stcm_instruction_item;

}


const char ::Neptools::Stcm::InstructionItem::TYPE_NAME[] = "neptools.stcm.instruction_item";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param48
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param48>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param48>::Make<LuaGetRef<::Neptools::Context &>, LuaGetRef<::uint32_t>>
    >("new");
    bld.AddFunction<
        static_cast<::uint32_t (::Neptools::Stcm::InstructionItem::Param48::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param48::Dump)
    >("dump");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48::Type (::Neptools::Stcm::InstructionItem::Param48::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param48::GetType)
    >("get_type");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>)
    >("get_read_4ac");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>)
    >("get_read_stack");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>)
    >("get_indirect");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>)
    >("get_immediate");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param48>().Get<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>()) (::Neptools::Stcm::InstructionItem::Param48::*)() const>(&::Neptools::Stcm::InstructionItem::Param48::Get<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>)
    >("get_mem_offset");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC>)
    >("new_read_4ac");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK>)
    >("new_read_stack");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT>)
    >("new_indirect");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE>)
    >("new_immediate");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param48 (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET), ::Neptools::Stcm::InstructionItem::Param48::Variant>)>(::Neptools::Stcm::InstructionItem::Param48::New<Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET>)
    >("new_mem_offset");

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param48> reg_neptools_stcm_instruction_item_param48;

}


const char ::Neptools::Stcm::InstructionItem::Param48::TYPE_NAME[] = "neptools.stcm.instruction_item.param48";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param48.type
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param48::Type>(TypeBuilder& bld)
{

    bld.Add("MEM_OFFSET", ::Neptools::Stcm::InstructionItem::Param48::Type::MEM_OFFSET);
    bld.Add("IMMEDIATE", ::Neptools::Stcm::InstructionItem::Param48::Type::IMMEDIATE);
    bld.Add("INDIRECT", ::Neptools::Stcm::InstructionItem::Param48::Type::INDIRECT);
    bld.Add("READ_STACK", ::Neptools::Stcm::InstructionItem::Param48::Type::READ_STACK);
    bld.Add("READ_4AC", ::Neptools::Stcm::InstructionItem::Param48::Type::READ_4AC);

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param48::Type> reg_neptools_stcm_instruction_item_param48_type;

}

const char ::Neptools::Lua::TypeName<::Neptools::Stcm::InstructionItem::Param48::Type>::TYPE_NAME[] =
    "neptools.stcm.instruction_item.param48.type";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param>(TypeBuilder& bld)
{

    bld.AddFunction<
        static_cast<void (::Neptools::Stcm::InstructionItem::Param::*)(::Neptools::Sink &) const>(&::Neptools::Stcm::InstructionItem::Param::Dump)
    >("dump");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param::Type (::Neptools::Stcm::InstructionItem::Param::*)() const noexcept>(&::Neptools::Stcm::InstructionItem::Param::GetType)
    >("get_type");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>)
    >("get_coll_link");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>)
    >("get_instr_ptr1");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>)
    >("get_instr_ptr0");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>)
    >("get_read_4ac");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>)
    >("get_read_stack");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>)
    >("get_indirect");
    bld.AddFunction<
        static_cast<decltype(std::declval<::Neptools::Stcm::InstructionItem::Param>().Get<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>()) (::Neptools::Stcm::InstructionItem::Param::*)() const>(&::Neptools::Stcm::InstructionItem::Param::Get<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>)
    >("get_mem_offset");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK>)
    >("new_coll_link");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1>)
    >("new_instr_ptr1");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0>)
    >("new_instr_ptr0");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::READ_4AC), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::READ_4AC>)
    >("new_read_4ac");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::READ_STACK), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::READ_STACK>)
    >("new_read_stack");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::INDIRECT), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::INDIRECT>)
    >("new_indirect");
    bld.AddFunction<
        static_cast<::Neptools::Stcm::InstructionItem::Param (*)(std::variant_alternative_t<static_cast<size_t>(::Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET), ::Neptools::Stcm::InstructionItem::Param::Variant>)>(::Neptools::Stcm::InstructionItem::Param::New<Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET>)
    >("new_mem_offset");

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param> reg_neptools_stcm_instruction_item_param;

}


const char ::Neptools::Stcm::InstructionItem::Param::TYPE_NAME[] = "neptools.stcm.instruction_item.param";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param.mem_offset
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param::MemOffset>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Neptools::NotNull<::Neptools::LabelPtr>, &::Neptools::Stcm::InstructionItem::Param::MemOffset::target>
    >("get_target");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::MemOffset::param_4>
    >("get_param_4");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::MemOffset, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::MemOffset::param_8>
    >("get_param_8");
    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param::MemOffset>::Make<LuaGetRef<::Neptools::NotNull<::Neptools::LabelPtr>>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::MemOffset> reg_neptools_stcm_instruction_item_param_mem_offset;

}


const char ::Neptools::Stcm::InstructionItem::Param::MemOffset::TYPE_NAME[] = "neptools.stcm.instruction_item.param.mem_offset";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param.indirect
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param::Indirect>(TypeBuilder& bld)
{

    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::Indirect, ::uint32_t, &::Neptools::Stcm::InstructionItem::Param::Indirect::param_0>
    >("get_param_0");
    bld.AddFunction<
        &::Neptools::Lua::GetMember<::Neptools::Stcm::InstructionItem::Param::Indirect, ::Neptools::Stcm::InstructionItem::Param48, &::Neptools::Stcm::InstructionItem::Param::Indirect::param_8>
    >("get_param_8");
    bld.AddFunction<
        &::Neptools::Lua::TypeTraits<::Neptools::Stcm::InstructionItem::Param::Indirect>::Make<LuaGetRef<::uint32_t>, LuaGetRef<::Neptools::Stcm::InstructionItem::Param48>>
    >("new");

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::Indirect> reg_neptools_stcm_instruction_item_param_indirect;

}


const char ::Neptools::Stcm::InstructionItem::Param::Indirect::TYPE_NAME[] = "neptools.stcm.instruction_item.param.indirect";

namespace Neptools::Lua
{

// class neptools.stcm.instruction_item.param.type
template<>
void TypeRegister::DoRegister<::Neptools::Stcm::InstructionItem::Param::Type>(TypeBuilder& bld)
{

    bld.Add("MEM_OFFSET", ::Neptools::Stcm::InstructionItem::Param::Type::MEM_OFFSET);
    bld.Add("INDIRECT", ::Neptools::Stcm::InstructionItem::Param::Type::INDIRECT);
    bld.Add("READ_STACK", ::Neptools::Stcm::InstructionItem::Param::Type::READ_STACK);
    bld.Add("READ_4AC", ::Neptools::Stcm::InstructionItem::Param::Type::READ_4AC);
    bld.Add("INSTR_PTR0", ::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR0);
    bld.Add("INSTR_PTR1", ::Neptools::Stcm::InstructionItem::Param::Type::INSTR_PTR1);
    bld.Add("COLL_LINK", ::Neptools::Stcm::InstructionItem::Param::Type::COLL_LINK);

}
static TypeRegister::StateRegister<::Neptools::Stcm::InstructionItem::Param::Type> reg_neptools_stcm_instruction_item_param_type;

}

const char ::Neptools::Lua::TypeName<::Neptools::Stcm::InstructionItem::Param::Type>::TYPE_NAME[] =
    "neptools.stcm.instruction_item.param.type";

#endif
