#ifndef UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#define UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#pragma once

#include "not_null.hpp"

#include <atomic>

namespace Libshit
{

  class RefCounted
  {
  public:
    RefCounted() = default;
    RefCounted(const RefCounted&) = delete;
    void operator=(const RefCounted&) = delete;
    virtual ~RefCounted() = default;

    virtual void Dispose() noexcept {}

    size_t use_count() const // emulate boost refcount
    { return strong_count.load(std::memory_order_relaxed); }
    size_t weak_use_count() const
    { return weak_count.load(std::memory_order_relaxed); }

    void AddRef()
    {
      LIBSHIT_ASSERT(use_count() >= 1);
      strong_count.fetch_add(1, std::memory_order_relaxed);
    }
    void RemoveRef()
    {
      if (strong_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
      {
        LIBSHIT_ASSERT(weak_use_count() > 0);
        Dispose();
        LIBSHIT_ASSERT(weak_use_count() > 0);
        RemoveWeakRef();
      }
    }

    void AddWeakRef() { weak_count.fetch_add(1, std::memory_order_relaxed); }
    void RemoveWeakRef()
    {
      if (weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
        delete this;
    }
    bool LockWeak()
    {
      size_t count = strong_count.load(std::memory_order_relaxed);
      do
        if (count == 0) return false;
      while (!strong_count.compare_exchange_weak(
               count, count+1,
               std::memory_order_acq_rel, std::memory_order_relaxed));
      return true;
    }

  private:
    // every object has an implicit weak_count, removed when removing last
    // strong ref
    std::atomic<size_t> weak_count{1}, strong_count{1};
  };

  template <typename T>
  constexpr bool IS_REFCOUNTED = std::is_base_of<RefCounted, T>::value;

  // RefCounted objects only need one pointer, others need two
  template <typename T>
  struct SharedPtrStorageNormal
  {
    SharedPtrStorageNormal() = default;
    SharedPtrStorageNormal(RefCounted* ctrl, T* ptr) : ctrl{ctrl}, ptr{ptr} {}

    RefCounted* GetCtrl() const noexcept { return ctrl; }
    T* GetPtr() const noexcept { return ptr; }
    void Reset() noexcept { ctrl = nullptr; ptr = nullptr; }
    void Swap(SharedPtrStorageNormal& o) noexcept
    {
      std::swap(ctrl, o.ctrl);
      std::swap(ptr, o.ptr);
    }

    RefCounted* ctrl = nullptr;
    T* ptr = nullptr;
  };

  template <typename T>
  struct SharedPtrStorageRefCounted
  {
    using U = std::remove_const_t<T>;

    // can't put the IS_REFCOUNTED static assert into the class body, as that
    // would break when using this on incomplete types
    SharedPtrStorageRefCounted()
    { static_assert(IS_REFCOUNTED<T>); }
    SharedPtrStorageRefCounted(RefCounted* ctrl, T* ptr)
      : ptr{const_cast<U*>(ptr)}
    {
      (void) ctrl;
      static_assert(IS_REFCOUNTED<T>);
      LIBSHIT_ASSERT(ctrl == ptr);
    }

    RefCounted* GetCtrl() const noexcept
    {
      static_assert(IS_REFCOUNTED<T>);
      return ptr;
    }
    T* GetPtr() const noexcept { return ptr; }
    void Reset() noexcept { ptr = nullptr; }
    void Swap(SharedPtrStorageRefCounted& o) noexcept { std::swap(ptr, o.ptr); }

    U* ptr = nullptr;
  };

  template <typename T, template<typename> class Storage> class WeakPtrBase;

  // shared (strong) ptr
  template <typename T, template<typename> class Storage>
  class SharedPtrBase
  {
    template <typename U = T>
    static constexpr bool IS_NORMAL_S = std::is_same<
      Storage<T>, SharedPtrStorageNormal<U>>::value;
    template <typename U = T>
    static constexpr bool IS_REFCOUNTED_S = std::is_same<
      Storage<T>, SharedPtrStorageRefCounted<U>>::value;
    static_assert(IS_NORMAL_S<> || IS_REFCOUNTED_S<>);

  public:
    SharedPtrBase() = default;
    SharedPtrBase(std::nullptr_t) noexcept {}

    // alias ctor
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    SharedPtrBase(SharedPtrBase<U, Storage> o, T* ptr) noexcept
      : s{o.GetCtrl(), ptr}
    { o.s.Reset(); }

    // weak->strong, throws on error
    template <typename U>
    SharedPtrBase(const WeakPtrBase<U, Storage>& o);

    // copy/move/conv ctor
    SharedPtrBase(const SharedPtrBase& o) noexcept
      : SharedPtrBase{o.GetCtrl(), o.get(), true} {}
    template <typename U>
    SharedPtrBase(const SharedPtrBase<U, Storage>& o) noexcept
      : SharedPtrBase{o.GetCtrl(), o.get(), true} {}
    // refcounted->refcounted/shared ok
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    SharedPtrBase(const SharedPtrBase<U, SharedPtrStorageRefCounted>& o) noexcept
      : SharedPtrBase{o.GetCtrl(), o.get(), true} {}

    SharedPtrBase(SharedPtrBase&& o) noexcept
      : s{o.GetCtrl(), o.get()}
    { o.s.Reset(); }
    template <typename U>
    SharedPtrBase(SharedPtrBase<U, Storage>&& o) noexcept
      : s{o.GetCtrl(), o.get()}
    { o.s.Reset(); }
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    SharedPtrBase(SharedPtrBase<U, SharedPtrStorageRefCounted>&& o) noexcept
      : s{o.GetCtrl(), o.get()}
    { o.s.Reset(); }


    // raw ptr for RefCounted objects
    template <typename U, typename V = T,
              typename = std::enable_if_t<IS_REFCOUNTED_S<V>>>
    SharedPtrBase(U* ptr, bool add_ref = true) noexcept
      : SharedPtrBase{const_cast<std::remove_const_t<U>*>(ptr), ptr, add_ref} {}

    // misc standard members
    SharedPtrBase& operator=(SharedPtrBase p) noexcept
    {
      s.Swap(p.s);
      return *this;
    }
    ~SharedPtrBase() noexcept
    { if (GetCtrl()) GetCtrl()->RemoveRef(); }

    // shared_ptr members
    void reset() noexcept { SharedPtrBase{}.swap(*this); }
    // no other reset functions: currently only make_shared like usage supported
    void swap(SharedPtrBase& o) noexcept { s.Swap(o.s); }

    T* get() const noexcept { return s.GetPtr(); }
    T& operator*() const noexcept { return *s.GetPtr(); }
    T* operator->() const noexcept { return s.GetPtr(); }
    size_t use_count() const noexcept
    { return s.GetCtrl() ? s.GetCtrl()->use_count() : 0; }
    bool unique() const noexcept { return use_count() == 1; }
    explicit operator bool() const noexcept { return s.GetPtr(); }

    // casts
#define LIBSHIT_GEN(camel, snake)                        \
    template <typename U>                                \
    friend SharedPtrBase<U, Storage> camel##PointerCast( \
      const SharedPtrBase& p) noexcept                   \
    { return {p.GetCtrl(), snake##_cast<U*>(p.get()), true}; }
    LIBSHIT_GEN(Static, static)
    LIBSHIT_GEN(Dynamic, dynamic)
    LIBSHIT_GEN(Const, const)
#undef LIBSHIT_GEN

    // low level stuff
    SharedPtrBase(RefCounted* ctrl, T* ptr, bool incr) noexcept
      : s{ctrl, ptr}
    { if (incr && ctrl) ctrl->AddRef(); }

    RefCounted* GetCtrl() const noexcept { return s.GetCtrl(); }

  private:
    Storage<T> s;

    template <typename U, template<typename> class UStorage>
    friend class SharedPtrBase;
  };

  template <typename T, template<typename> class Storage>
  inline void swap(
    SharedPtrBase<T, Storage>& a, SharedPtrBase<T, Storage>& b) noexcept
  { a.swap(b); }

  // comparison operators
#define LIBSHIT_GEN(type, get, op)                               \
  template <typename T, template<typename> class TStorage,       \
            typename U, template<typename> class UStorage>       \
  inline bool operator op(                                       \
    const type##PtrBase<T, TStorage>& a,                         \
    const type##PtrBase<U, UStorage>& b) noexcept                \
  { return a.get() op b.get(); }                                 \
  template <typename T, template<typename> class Storage>        \
  inline bool operator op(                                       \
    const type##PtrBase<T, Storage>& p, std::nullptr_t) noexcept \
  { return p.get() op nullptr; }                                 \
  template <typename T, template<typename> class Storage>        \
  inline bool operator op(                                       \
    std::nullptr_t, const type##PtrBase<T, Storage>& p) noexcept \
  { return nullptr op p.get(); }
#define LIBSHIT_GEN2(type, get)                         \
  LIBSHIT_GEN(type, get, ==) LIBSHIT_GEN(type, get, !=) \
  LIBSHIT_GEN(type, get, <)  LIBSHIT_GEN(type, get, <=) \
  LIBSHIT_GEN(type, get, >)  LIBSHIT_GEN(type, get, >=)
  LIBSHIT_GEN2(Shared, get)

  // use these types. usually SmartPtr; use SharedPtr when you need aliasing
  // with an otherwise RefCounted type
  template <typename T>
  using SharedPtr = SharedPtrBase<T, SharedPtrStorageNormal>;
  template <typename T>
  using RefCountedPtr = SharedPtrBase<T, SharedPtrStorageRefCounted>;
  template <typename T>
  using SmartPtr = std::conditional_t<
    IS_REFCOUNTED<T>,
    SharedPtrBase<T, SharedPtrStorageRefCounted>,
    SharedPtrBase<T, SharedPtrStorageNormal>>;


  // weak ptr
  template <typename T, template<typename> class Storage>
  class WeakPtrBase
  {
    template <typename U = T>
    static constexpr bool IS_NORMAL_S = std::is_same<
      Storage<T>, SharedPtrStorageNormal<U>>::value;
    template <typename U = T>
    static constexpr bool IS_REFCOUNTED_S = std::is_same<
      Storage<T>, SharedPtrStorageRefCounted<U>>::value;
    static_assert(IS_NORMAL_S<> || IS_REFCOUNTED_S<>);

  public:
    WeakPtrBase() = default;
    WeakPtrBase(std::nullptr_t) noexcept {}

    template <typename U>
    WeakPtrBase(const SharedPtrBase<U, Storage>& o) noexcept
      : WeakPtrBase{o.GetCtrl(), o.get(), true} {}
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    WeakPtrBase(const SharedPtrBase<U, SharedPtrStorageRefCounted>& o) noexcept
      : WeakPtrBase{o.GetCtrl(), o.get(), true} {}

    // copy/move/conv ctor
    WeakPtrBase(const WeakPtrBase& o) noexcept
      : WeakPtrBase{o.s.GetCtrl(), o.s.GetPtr(), true} {}
    template <typename U>
    WeakPtrBase(const WeakPtrBase<U, Storage>& o) noexcept
      : WeakPtrBase{o.s.GetCtrl(), o.s.GetPtr(), true} {}
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    WeakPtrBase(const WeakPtrBase<U, SharedPtrStorageRefCounted>& o) noexcept
      : WeakPtrBase{o.s.GetCtrl(), o.s.GetPtr(), true} {}

    WeakPtrBase(WeakPtrBase&& o) noexcept : s{o.s.GetCtrl(), o.s.GetPtr()}
    { o.s.Reset(); }
    template <typename U>
    WeakPtrBase(WeakPtrBase<U, Storage>&& o) noexcept
      : s{o.s.GetCtrl(), o.s.GetPtr()}
    { o.s.Reset(); }
    template <typename U,
              typename V = T,
              typename = std::enable_if_t<IS_NORMAL_S<V>>>
    WeakPtrBase(WeakPtrBase<U, SharedPtrStorageRefCounted>&& o) noexcept
      : s{o.s.GetCtrl(), o.s.GetPtr()}
    { o.s.Reset(); }

    // raw ptr for RefCounted objects
    // assume there's a strong reference to it...
    template <typename U, typename V = T,
              typename = std::enable_if_t<IS_REFCOUNTED_S<V>>>
    WeakPtrBase(U* ptr, bool add_ref = true) noexcept
      : WeakPtrBase{const_cast<std::remove_const_t<U>*>(ptr), ptr, add_ref} {}

    // misc standard members
    WeakPtrBase& operator=(WeakPtrBase o) noexcept
    {
      s.Swap(o.s);
      return *this;
    }
    ~WeakPtrBase() noexcept
    {
      if (s.GetCtrl()) s.GetCtrl()->RemoveWeakRef();
    }

    // weak_ptr members
    void reset() noexcept { WeakPtrBase{}.swap(*this); }
    void swap(WeakPtrBase& o) noexcept { s.Swap(o.s); }

    size_t use_count() const noexcept
    { return s.GetCtrl() ? s.GetCtrl()->use_count() : 0; }
    bool expired() const noexcept { return use_count() == 0; }

    SharedPtrBase<T, Storage> lock() const noexcept
    {
      auto ctrl = s.GetCtrl();
      if (ctrl && ctrl->LockWeak()) return {ctrl, s.GetPtr(), false};
      else return {};
    }

    // not in weak_ptr
    SharedPtrBase<T, Storage> unsafe_lock() const noexcept
    {
      // not bulletproof, but should catch most problems
      LIBSHIT_ASSERT(!expired());
      return {s.GetCtrl(), s.GetPtr(), true};
    }

    T* unsafe_get() const noexcept
    {
      LIBSHIT_ASSERT(!expired());
      return s.GetPtr();
    }

    // low level stuff
    WeakPtrBase(RefCounted* ctrl, T* ptr, bool incr) noexcept
      : s{ctrl, ptr}
    { if (incr && ctrl) ctrl->AddWeakRef(); }

    RefCounted* GetCtrl() const noexcept { return s.GetCtrl(); }
    T* GetPtr() const noexcept { return s.GetPtr(); }

  private:
    Storage<T> s;

    template <typename U, template<typename> class UStorage>
    friend class WeakPtrBase;
  };

  LIBSHIT_GEN2(Weak, GetPtr)
#undef LIBSHIT_GEN2
#undef LIBSHIT_GEN

  template <typename T, template<typename> class Storage>
  inline void swap(
    WeakPtrBase<T, Storage>& a, WeakPtrBase<T, Storage>& b) noexcept
  { a.swap(b); }


  template <typename T>
  using WeakPtr = WeakPtrBase<T, SharedPtrStorageNormal>;
  template <typename T>
  using WeakRefCountedPtr = WeakPtrBase<T, SharedPtrStorageRefCounted>;
  template <typename T>
  using WeakSmartPtr = std::conditional_t<
    IS_REFCOUNTED<T>,
    WeakPtrBase<T, SharedPtrStorageRefCounted>,
    WeakPtrBase<T, SharedPtrStorageNormal>>;


  template <typename T, template<typename> class Storage>
  template <typename U>
  inline SharedPtrBase<T, Storage>::SharedPtrBase(
    const WeakPtrBase<U, Storage>& o)
    : s{o.GetCtrl(), o.unsafe_get()}
  {
    if (!s.GetCtrl() || !s.GetCtrl()->LockWeak())
      LIBSHIT_THROW(std::bad_weak_ptr{});
  }

  // make_shared like helper
  template <typename T, typename Ret, typename = void>
  struct MakeSharedHelper
  {
    struct Alloc : public RefCounted
    {
      alignas(T) char t[sizeof(T)];

      template <typename... Args>
      Alloc(Args&&... args)
      { new (t) T(std::forward<Args>(args)...); }

      T* Get() noexcept { return reinterpret_cast<T*>(t); }
      void Dispose() noexcept override { Get()->~T(); }
    };

    template <typename... Args>
    static NotNull<Ret> Make(Args&&... args)
    {
      auto a = new Alloc{std::forward<Args>(args)...};
      return NotNull<Ret>{a, a->Get(), false};
    }
  };

  template <typename T>
  struct MakeSharedHelper<T, RefCountedPtr<T>>
  {
    template <typename... Args>
    static NotNull<RefCountedPtr<T>> Make(Args&&... args)
    {
      return NotNull<RefCountedPtr<T>>{
        new T(std::forward<Args>(args)...), false};
    }
  };


#define LIBSHIT_GEN(variant)                             \
  template <typename T, typename... Args>                \
  NotNull<variant##Ptr<T>> Make##variant(Args&&... args) \
  {                                                      \
    return MakeSharedHelper<T, variant##Ptr<T>>::Make(   \
      std::forward<Args>(args)...);                      \
  }
  LIBSHIT_GEN(Shared) LIBSHIT_GEN(RefCounted) LIBSHIT_GEN(Smart)
#undef LIBSHIT_GEN

}

#endif
