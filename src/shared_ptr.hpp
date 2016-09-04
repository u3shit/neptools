#ifndef UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#define UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#pragma once

#include <atomic>
#include <boost/intrusive_ptr.hpp>
#include "not_null.hpp"

namespace Neptools
{

template <typename T, typename = void> struct MakeSharedHelper;

class RefCounted
{
public:
    RefCounted() = default;
    RefCounted(const RefCounted&) = delete;
    void operator=(const RefCounted&) = delete;
    virtual ~RefCounted() = default;

    size_t use_count() const { return refcount; } // emulate boost refcount

private:
    mutable std::atomic<size_t> refcount{0};
    friend void intrusive_ptr_add_ref(const RefCounted* rc) noexcept
    { ++rc->refcount; }
    friend void intrusive_ptr_release(const RefCounted* rc) noexcept
    {
        if (--rc->refcount == 0) delete rc;
    }

    template <typename T, typename> friend struct MakeSharedHelper;
    explicit RefCounted(size_t rc_init) : refcount{rc_init} {}
};

template<typename T, typename... Args>
inline boost::intrusive_ptr<T> MakeRefCounted(Args&&... args)
{
    NEPTOOLS_STATIC_ASSERT(std::is_base_of<RefCounted, T>::value);
    return {new T(std::forward<Args>(args)...)};
}

template <typename T>
class SharedPtr
{
public:
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) noexcept {}

    // alias ctor
    template <typename U>
    SharedPtr(SharedPtr<U> o, T* ptr) noexcept : ctrl{o.ctrl}, ptr{ptr}
    { o.ctrl = nullptr; }

    // copy/move/conv ctor
    SharedPtr(const SharedPtr& o) noexcept : SharedPtr{o.ctrl, o.ptr, true} {}
    template <typename U>
    SharedPtr(const SharedPtr<U>& o) noexcept : SharedPtr{o.ctrl, o.ptr, true} {}

    SharedPtr(SharedPtr&& o) noexcept : ctrl{o.ctrl}, ptr{o.ptr}
    { o.ctrl = nullptr; o.ptr = nullptr; }
    template <typename U>
    SharedPtr(SharedPtr<U>&& o) noexcept : ctrl{o.ctrl}, ptr{o.ptr}
    { o.ctrl = nullptr; o.ptr = nullptr; }

    // raw ptr for RefCounted objects
    template <typename U, typename =
              std::enable_if_t<std::is_base_of<RefCounted, U>::value>>
    SharedPtr(U* ptr, bool add_ref = true) noexcept
        : SharedPtr{static_cast<RefCounted*>(ptr), ptr, add_ref} {}

    template <typename U, typename =
              std::enable_if_t<std::is_base_of<RefCounted, U>::value>>
    SharedPtr(boost::intrusive_ptr<U> ptr) noexcept
        : SharedPtr{ptr.detach(), false} {}

    SharedPtr& operator=(SharedPtr p) noexcept
    {
        std::swap(ctrl, p.ctrl);
        std::swap(ptr, p.ptr);
        return *this;
    }
    ~SharedPtr() noexcept
    { if (ctrl) intrusive_ptr_release(ctrl); }

    template <typename U>
    operator boost::intrusive_ptr<U>() const & noexcept
    {
        CheckRefCounted(); CheckRefCounted<U>();
        NEPTOOLS_ASSERT(ctrl == ptr);
        return {ptr};
    }

    template <typename U>
    operator boost::intrusive_ptr<U>() && noexcept
    {
        CheckRefCounted(); CheckRefCounted<U>();
        NEPTOOLS_ASSERT(ctrl == ptr);
        auto ret = ptr;
        ctrl = ptr = nullptr;
        return {ret, false};
    }

    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    size_t use_count() const { return ctrl ? ctrl->use_count() : 0; }
    bool unique() const { return use_count() == 1; }
    explicit operator bool() const { return ptr; }

#define NEPTOOLS_GEN(camel, snake)                                       \
    template <typename U>                                                \
    friend SharedPtr<U> camel##PointerCast(const SharedPtr& p) noexcept  \
    { return {p.ctrl, snake##_cast<U*>(p.ptr), true}; }
    NEPTOOLS_GEN(Static, static)
    NEPTOOLS_GEN(Dynamic, dynamic)
    NEPTOOLS_GEN(Const, const)
#undef NEPTOOLS_GEN

    // low level stuff
    SharedPtr(RefCounted* ctrl, T* ptr, bool incr) noexcept
        : ctrl{ctrl}, ptr{ptr}
    { if (incr && ctrl) intrusive_ptr_add_ref(ctrl); }

    RefCounted* GetCtrl() const noexcept { return ctrl; }

private:
    template <typename U = T>
    static constexpr void CheckRefCounted()
    { static_assert(std::is_base_of<RefCounted, U>::value, "Need RefCounted object"); }

    RefCounted* ctrl = nullptr;
    T* ptr = nullptr;

    template <typename U> friend class SharedPtr;
};

#define NEPTOOLS_GEN(op)                                                        \
    template <typename T, typename U>                                           \
    inline bool operator op(const SharedPtr<T>& a, const SharedPtr<U>& b) noexcept \
    { return a.get() op b.get(); }                                              \
    template <typename T>                                                       \
    inline bool operator op(const SharedPtr<T>& p, std::nullptr_t) noexcept     \
    { return p.get() op nullptr; }                                              \
    template <typename T>                                                       \
    inline bool operator op(std::nullptr_t, const SharedPtr<T>& p) noexcept     \
    { return nullptr op p.get(); }
NEPTOOLS_GEN(==) NEPTOOLS_GEN(!=) NEPTOOLS_GEN(<) NEPTOOLS_GEN(<=)
NEPTOOLS_GEN(>) NEPTOOLS_GEN(>=)
#undef NEPTOOLS_GEN


template <typename T>
struct MakeSharedHelper<T, std::enable_if_t<std::is_base_of<RefCounted, T>::value>>
{
    template <typename... Args>
    static NotNull<SharedPtr<T>> Make(Args&&... args)
    { return NotNull<SharedPtr<T>>{new T(std::forward<Args>(args)...)}; }
};

template <typename T>
struct MakeSharedHelper<T, std::enable_if_t<!std::is_base_of<RefCounted, T>::value>>
{
    struct Alloc : public RefCounted
    {
        T t;
        template <typename... Args>
        Alloc(Args&&... args) : RefCounted{1}, t(std::forward<Args>(args)...) {}
    };

    template <typename... Args>
    static NotNull<SharedPtr<T>> Make(Args&&... args)
    {
        auto a = new Alloc{std::forward<Args>(args)...};
        return NotNull<SharedPtr<T>>{a, &a->t, false};
    }
};

template <typename T, typename... Args>
NotNull<SharedPtr<T>> MakeShared(Args&&... args)
{ return MakeSharedHelper<T>::Make(std::forward<Args>(args)...); }

}

#endif
