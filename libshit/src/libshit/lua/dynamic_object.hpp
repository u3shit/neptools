#ifndef UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#define UUID_0863E64B_09C6_449B_A18D_EFD42D57C604
#pragma once

#ifdef LIBSHIT_WITHOUT_LUA

namespace Libshit::Lua
{

  struct SmartObject {};
  class DynamicObject
  {
  public:
    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;
  };

#define LIBSHIT_DYNAMIC_OBJ_GEN private: static void dummy_function()
#define LIBSHIT_DYNAMIC_OBJECT LIBSHIT_DYNAMIC_OBJ_GEN

  template <typename T, typename Enable = void> struct SmartObjectMaker;

}

#else

#include "userdata.hpp"
#include "../meta.hpp"
#include "../not_null.hpp"
#include "../shared_ptr.hpp"

#include <type_traits>

namespace Libshit::Lua
{

  class LIBSHIT_LUAGEN(no_inherit=true) SmartObject {};

  // specialize if needed
  template <typename T, typename Enable = void>
  struct IsSmartObject : std::is_base_of<SmartObject, T> {};

  template <typename T>
  constexpr bool IS_SMART_OBJECT = IsSmartObject<T>::value;


  class LIBSHIT_LUAGEN(no_inherit=true,smart_object=true) DynamicObject
    : public SmartObject
  {
  public:
    DynamicObject() = default;
    DynamicObject(const DynamicObject&) = delete;
    void operator=(const DynamicObject&) = delete;
    virtual ~DynamicObject() = default;

    virtual void PushLua(StateRef vm, RefCounted& ctrl) = 0;
  };

  template <typename T>
  constexpr bool IS_SELF_PUSHABLE_DYNAMIC_OBJECT =
    std::is_base_of_v<DynamicObject, T> && std::is_base_of_v<RefCounted, T>;

#define LIBSHIT_THIS_TYPE std::remove_pointer_t<decltype(this)>
#define LIBSHIT_DYNAMIC_OBJ_GEN                                           \
  private:                                                                \
  void PushLua(::Libshit::Lua::StateRef vm,                               \
               ::Libshit::RefCounted& ctrl) override                      \
  {                                                                       \
    ::Libshit::Lua::Userdata::Cached::Create<::Libshit::SharedPtr<char>>( \
      vm, this, ::Libshit::Lua::TYPE_NAME<LIBSHIT_THIS_TYPE>, &ctrl,      \
      reinterpret_cast<char*>(this), true);                               \
  }

#define LIBSHIT_DYNAMIC_OBJECT \
  LIBSHIT_LUA_CLASS;           \
  LIBSHIT_DYNAMIC_OBJ_GEN

  inline DynamicObject& GetDynamicObject(DynamicObject& obj) { return obj; }

  namespace Detail
  {
    template <typename T, typename Enable = void>
    struct SmartPush
    {
      static void Push(StateRef& vm, RefCounted& ctrl, T& obj)
      {
        Userdata::Cached::Create<SharedPtr<char>>(
          vm, &obj, TYPE_NAME<T>, &ctrl, reinterpret_cast<char*>(&obj), true);
      }
    };

    template <typename T>
    struct SmartPush<
      T, std::enable_if_t<std::is_base_of<DynamicObject, T>::value>>
    {
      static void Push(StateRef& vm, RefCounted& ctrl, T& obj)
      { GetDynamicObject(obj).PushLua(vm, ctrl); }
    };
  }

  template <typename T, typename Enable = void>
  struct SmartObjectMaker : MakeSharedHelper<T, SmartPtr<T>> {};

  template <typename T>
  struct TypeTraits<T, std::enable_if_t<IS_SMART_OBJECT<T>>>
    : SmartObjectMaker<T>
  {
    using RawType = T;

    template <bool Unsafe>
    static T& Get(StateRef vm, bool arg, int idx)
    {
      return *Userdata::GetInherited<Unsafe, SharedPtr<char>, T>(
        vm, arg, idx).second;
    }

    static bool Is(StateRef vm, int idx)
    { return Userdata::IsInherited(vm, idx, TYPE_NAME<T>); }

    static void Push(StateRef vm, T& obj)
    { GetDynamicObject(obj).PushLua(vm, obj); }

    static void PrintName(std::ostream& os) { os << TYPE_NAME<T>; }
    static constexpr const char* TAG = TYPE_NAME<T>;
  };

  template <typename T>
  struct UserTypeTraits<T, std::enable_if_t<IS_SMART_OBJECT<T>>>
  {
    static constexpr bool INSTANTIABLE = true;
    static constexpr bool NEEDS_GC = true;
    static constexpr auto GcFun = Userdata::Cached::GcFun<
      SharedPtr<char>, T>;
  };

  template <typename T, template<typename> class Storage>
  struct TypeTraits<NotNull<SharedPtrBase<T, Storage>>,
                    std::enable_if_t<IsSmartObject<T>::value>> : TypeTraits<T>
  {
    using Ptr = NotNull<SharedPtrBase<T, Storage>>;

    template <bool Unsafe>
    static Ptr Get(StateRef vm, bool arg, int idx)
    {
      auto x = Userdata::GetInherited<Unsafe, SharedPtr<char>, T>(vm, arg, idx);
      return Ptr{x.first->GetCtrl(), x.second, true};
    }

    static void Push(StateRef vm, const Ptr& obj)
    { Detail::SmartPush<T>::Push(vm, *obj.Get().GetCtrl(), *obj); }
  };

  template <typename T, template<typename> class Storage>
  struct TypeTraits<SharedPtrBase<T, Storage>,
                    std::enable_if_t<IsSmartObject<T>::value>>
    : NullableTypeTraits<SharedPtrBase<T, Storage>> {};

  template <typename T, template<typename> class Storage>
  struct TypeTraits<WeakPtrBase<T, Storage>,
                    std::enable_if_t<IsSmartObject<T>::value>>
  {
    using Type = WeakPtrBase<T, Storage>;
    using RawType = T;

    template <bool Unsafe>
    static Type Get(StateRef vm, bool arg, int idx)
    {
      if (lua_isnil(vm, idx)) return nullptr;
      auto x = Userdata::GetInherited<Unsafe, SharedPtr<char>, T>(vm, arg, idx);
      return {x.first->GetCtrl(), x.second, true};
    }

    static bool Is(StateRef vm, int idx)
    { return lua_isnil(vm, idx) || Userdata::IsInherited(vm, idx, TYPE_NAME<T>); }

    static void Push(StateRef vm, const Type& obj)
    {
      if (auto sptr = obj.lock())
        Detail::SmartPush<T>::Push(vm, *sptr.GetCtrl(), *sptr);
      else lua_pushnil(vm);
    }

    static void PrintName(std::ostream& os) { os << TYPE_NAME<T>; }
    static constexpr const char* TAG = TYPE_NAME<T>;
  };

  template <typename Class, typename T, T Class::* Member>
  BOOST_FORCEINLINE NotNull<SharedPtr<T>> GetRefCountedOwnedMember(Class& cls)
  { return NotNull<SharedPtr<T>>{&cls, &(cls.*Member), true}; }

  template <typename Class, typename T, T Class::* Member>
  BOOST_FORCEINLINE NotNull<SharedPtr<T>>
  GetSmartOwnedMember(const NotNull<SharedPtr<Class>>& cls)
  { return NotNull<SharedPtr<T>>{cls.Get(), &(cls.get()->*Member)}; }

}

#endif
#endif
