#ifndef UUID_98C06485_8AE9_47DA_B99F_62CA5AF00FF4
#define UUID_98C06485_8AE9_47DA_B99F_62CA5AF00FF4
#pragma once

#include "utils.hpp"

#include <libshit/meta.hpp>
#include <libshit/lua/intrusive_object.hpp>
#include <libshit/lua/value_object.hpp>

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <typeindex>
#include <vector>
#include <boost/intrusive_ptr.hpp>

namespace Neptools
{

template <typename Item, typename... Rest> struct IndexOf;
template <typename Item, typename... Rest>
struct IndexOf<Item, Item, Rest...>
    : std::integral_constant<size_t, 0> {};

template <typename Item, typename Head, typename... Rest>
struct IndexOf<Item, Head, Rest...>
    : std::integral_constant<size_t, 1 + IndexOf<Item, Rest...>::value> {};

template <typename Item, typename... Args>
constexpr auto IndexOfV = IndexOf<Item, Args...>::value;

NEPTOOLS_STATIC_ASSERT(IndexOfV<int, float, double, int> == 2);

template <typename... Args>
class LIBSHIT_LUAGEN(
    post_register = "::Neptools::DynamicStructLua</*$= template_args */>::Register(bld);")
DynamicStruct final : public Libshit::Lua::IntrusiveObject
{
    LIBSHIT_LUA_CLASS;
private:
    template <typename Ret, size_t I, typename T, typename... TRest,
              typename Thiz, typename Fun, typename... FunArgs>
    static Ret VisitHlp(Thiz thiz, size_t i, Fun&& fun, FunArgs&&... args)
    {
        if (thiz->type->items[i].idx == I)
            return fun(thiz->template Get<T>(i), thiz->GetSize(i),
                       std::forward<FunArgs>(args)...);
        else
            return VisitHlp<Ret, I+1, TRest...>(
                thiz, i, std::forward<Fun>(fun), std::forward<FunArgs>(args)...);
    }

    template <typename Ret, size_t I, typename Thiz, typename Fun,
              typename... FunArgs>
    static Ret VisitHlp(Thiz, size_t, Fun&&, FunArgs&&...) { abort(); }

public:
    template <typename T>
    LIBSHIT_NOLUA static constexpr size_t GetIndexFromType()
    { return IndexOfV<T, Args...>; }

    static constexpr const size_t SIZE_OF[] = { sizeof(Args)... };
    static constexpr const size_t ALIGN_OF[] = { alignof(Args)... };

    struct Type final : public Libshit::Lua::IntrusiveObject
    {
        LIBSHIT_LUA_CLASS;
    public:
        Type(const Type&) = delete;
        void operator=(const Type&) = delete;
        ~Type() = delete;

        LIBSHIT_NOLUA mutable std::atomic<size_t> refcount;
        LIBSHIT_LUAGEN(get=true) LIBSHIT_LUAGEN(get=true,name="__len")
        size_t item_count;
        LIBSHIT_LUAGEN(get=true) size_t byte_size;
        struct Item
        {
            size_t idx;
            size_t size;
            size_t offset;
        };
        LIBSHIT_NOLUA Item items[1];
    };
    static_assert(std::is_standard_layout_v<Type>);
    using TypePtr = boost::intrusive_ptr<const Type>;

    friend void intrusive_ptr_add_ref(const Type* t)
    { t->refcount.fetch_add(1, std::memory_order_relaxed); }

    friend void intrusive_ptr_release(const Type* t)
    {
        if (t->refcount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            ::operator delete(const_cast<Type*>(t));
    }

    class TypeBuilder final : public Libshit::Lua::ValueObject
    {
        LIBSHIT_LUA_CLASS;
    public:
        TypeBuilder() = default; // force lua ctor

        LIBSHIT_NOLUA auto& GetDesc() { return desc; }
        LIBSHIT_NOLUA const auto& GetDesc() const { return desc; }

        void Reserve(size_t size) { desc.reserve(size); }

        template <typename T>
        LIBSHIT_NOLUA void Add(size_t size = sizeof(T))
        { desc.emplace_back(IndexOfV<T, Args...>, size); }

        LIBSHIT_NOLUA void Add(size_t i, size_t size)
        {
            LIBSHIT_ASSERT_MSG(i < sizeof...(Args), "index out of range");
            desc.push_back({i, size});
        }

        TypePtr Build() const
        {
            auto ptr = operator new(
                sizeof(Type) + (desc.size() - 1) * sizeof(typename Type::Item));
            boost::intrusive_ptr<Type> ret{static_cast<Type*>(ptr), false};
            ret->refcount.store(1, std::memory_order_relaxed);
            ret->item_count = desc.size();

            size_t offs = 0;
            for (size_t i = 0; i < desc.size(); ++i)
            {
                ret->items[i].idx = desc[i].first;
                ret->items[i].size = desc[i].second;
                ret->items[i].offset = offs;

                offs += desc[i].second;
                auto al = ALIGN_OF[desc[i].first];
                offs = (offs + al - 1) / al * al;
            }
            ret->byte_size = offs;

            return ret;
        }

    private:
        std::vector<std::pair<size_t, size_t>> desc;
    };

    // actual class begin
    LIBSHIT_LUAGEN(result_type=
        "::boost::intrusive_ptr<::Neptools::DynamicStruct</*$= cls.template_args */>>")
    static boost::intrusive_ptr<DynamicStruct> New(TypePtr type)
    {
        auto ptr = ::operator new(sizeof(DynamicStruct) + type->byte_size - 1);
        try
        {
            auto obj = new (ptr) DynamicStruct{std::move(type)};
            return {obj, false};
        }
        catch (...)
        {
            ::operator delete(ptr);
            throw;
        }
    }

    DynamicStruct(const DynamicStruct&) = delete;
    void operator=(const DynamicStruct&) = delete;
    ~DynamicStruct()
    {
        if (type)
            ForEach(Destroy{});
    }

    LIBSHIT_LUAGEN() LIBSHIT_LUAGEN(name="__len")
    size_t GetSize() const noexcept { return type->item_count; }
    LIBSHIT_NOLUA size_t GetSize(size_t i) const noexcept
    {
        LIBSHIT_ASSERT_MSG(i < GetSize(), "index out of range");
        return type->items[i].size;
    }
    LIBSHIT_NOLUA size_t GetTypeIndex(size_t i) const noexcept
    {
        LIBSHIT_ASSERT_MSG(i < GetSize(), "index out of range");
        return type->items[i].idx;
    }

    template <typename T>
    LIBSHIT_NOLUA bool Is(size_t i) const noexcept
    {
        return GetTypeIndex(i) == GetIndexFromType<T>();
    }

    const TypePtr& GetType() const noexcept { return type; }
    LIBSHIT_NOLUA void* GetData() noexcept { return data; }
    LIBSHIT_NOLUA const void* GetData() const noexcept { return data; }

    LIBSHIT_NOLUA void* GetData(size_t i) noexcept
    {
        LIBSHIT_ASSERT_MSG(i <= GetSize(), "index out of range");
        return &data[type->items[i].offset];
    }
    LIBSHIT_NOLUA const void* GetData(size_t i) const noexcept
    {
        LIBSHIT_ASSERT_MSG(i <= GetSize(), "index out of range");
        return &data[type->items[i].offset];
    }

    template <typename T>
    LIBSHIT_NOLUA T& Get(size_t i) noexcept
    {
        LIBSHIT_ASSERT_MSG(Is<T>(i), "specified item is not T");
        return *reinterpret_cast<T*>(data + type->items[i].offset);
    }

    template <typename T>
    LIBSHIT_NOLUA const T& Get(size_t i) const noexcept
    {
        LIBSHIT_ASSERT_MSG(Is<T>(i), "specified item is not T");
        return *reinterpret_cast<const T*>(data + type->items[i].offset);
    }

    template <typename Ret = void, typename... FunArgs>
    LIBSHIT_NOLUA Ret Visit(size_t i, FunArgs&&... f)
    { return VisitHlp<Ret, 0, Args...>(this, i, std::forward<FunArgs>(f)...); }

    template <typename... FunArgs>
    LIBSHIT_NOLUA void ForEach(FunArgs&&... f)
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit(i, std::forward<FunArgs>(f)...);
    }

    // const version
    template <typename Ret = void, typename... FunArgs>
    LIBSHIT_NOLUA Ret Visit(size_t i, FunArgs&&... f) const
    { return VisitHlp<Ret, 0, Args...>(this, i, std::forward<FunArgs>(f)...); }

    template <typename... FunArgs>
    LIBSHIT_NOLUA void ForEach(FunArgs&&... args) const
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit(i, std::forward<FunArgs>(args)...);
    }

private:
    DynamicStruct(TypePtr type) : type{std::move(type)}
    {
        size_t i = 0;
        try
        {
            for (i = 0; i < this->type->item_count; ++i)
                Visit(i, Make{});
        }
        catch (...)
        {
            while (i > 0) Visit(--i, Destroy{});
            throw;
        }
    }

    struct Make
    {
        template <typename T>
        void operator()(T& x, size_t)
        { new (&x) T; }
    };

    struct Destroy
    {
        template <typename T>
        void operator()(T& x, size_t)
        { x.~T(); }
    };

    TypePtr type;
    mutable std::atomic<size_t> refcount{1};
    char data[1];

    friend void intrusive_ptr_add_ref(const DynamicStruct* t)
    { t->refcount.fetch_add(1, std::memory_order_relaxed); }

    friend void intrusive_ptr_release(const DynamicStruct* t)
    {
        if (t->refcount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            t->~DynamicStruct();
            ::operator delete(const_cast<DynamicStruct*>(t));
        }
    }
};

template <typename... Args>
constexpr const size_t DynamicStruct<Args...>::SIZE_OF[];

template <typename... Args>
constexpr const size_t DynamicStruct<Args...>::ALIGN_OF[];

}
#endif
