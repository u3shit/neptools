#ifndef UUID_98C06485_8AE9_47DA_B99F_62CA5AF00FF4
#define UUID_98C06485_8AE9_47DA_B99F_62CA5AF00FF4
#pragma once

#include "utils.hpp"
#include <atomic>
#include <iostream>
#include <cstdint>
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
class DynamicStruct
{
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
    static constexpr size_t GetIndexFromType()
    { return IndexOfV<T, Args...>; }

    static constexpr const size_t SIZE_OF[] = { sizeof(Args)... };
    static constexpr const size_t ALIGN_OF[] = { alignof(Args)... };

    struct Type
    {
        Type(const Type&) = delete;
        void operator=(const Type&) = delete;
        ~Type() = delete;

        mutable std::atomic<size_t> refcount;
        size_t item_count, size;
        struct Item
        {
            size_t idx;
            size_t size;
            size_t offset;
        };
        Item items[1];
    };
    using TypePtr = boost::intrusive_ptr<const Type>;

    friend void intrusive_ptr_add_ref(const Type* t)
    { ++t->refcount; }

    friend void intrusive_ptr_release(const Type* t)
    {
        if (--t->refcount == 0) // op-- == fetch_sub(1)-1
            ::operator delete(const_cast<Type*>(t));
    }

    class TypeBuilder
    {
    public:
        auto& GetDesc() { return desc; }
        const auto& GetDesc() const { return desc; }

        template <typename T>
        void Add(size_t size = sizeof(T))
        { desc.emplace_back(IndexOfV<T, Args...>, size); }

        void Add(size_t i, size_t size)
        {
            NEPTOOLS_ASSERT_MSG(i < sizeof...(Args), "index out of range");
            desc.push_back({i, size});
        }

        TypePtr Build() const
        {
            auto ptr = operator new(
                sizeof(Type) + (desc.size() - 1) * sizeof(typename Type::Item));
            boost::intrusive_ptr<Type> ret{static_cast<Type*>(ptr), false};
            ret->refcount = 1;
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
            ret->size = offs;

            return ret;
        }

    private:
        std::vector<std::pair<size_t, size_t>> desc;
    };

    // actual class begin
    DynamicStruct() = default;
    DynamicStruct(const TypePtr& type) : type{type}
    {
        data.reset(new char[type->size]);
        ForEach([](auto& x, size_t)
                { new (&x) std::remove_reference_t<decltype(x)>; });
    }

    DynamicStruct(DynamicStruct&& o) : type{o.type}, data{std::move(o.data)}
    { o.type = nullptr; o.data = nullptr; }
    DynamicStruct& operator=(DynamicStruct&& o)
    {
        if (this != &o)
        {
            type = o.type;
            data = std::move(o.data);
            o.type = nullptr;
            o.data = nullptr;
        }
        return *this;
    }
    ~DynamicStruct()
    {
        ForEach([](auto& x, size_t)
                {
                    using T = std::remove_reference_t<decltype(x)>;
                    x.~T();
                });
    }

    size_t GetSize() const { return type->item_count; }
    size_t GetSize(size_t i) const
    {
        NEPTOOLS_ASSERT_MSG(i < GetSize(), "index out of range");
        return type->items[i].size;
    }
    size_t GetTypeIndex(size_t i) const
    {
        NEPTOOLS_ASSERT_MSG(i < GetSize(), "index out of range");
        return type->items[i].idx;
    }

    template <typename T>
    bool Is(size_t i) const
    {
        return GetTypeIndex(i) == GetIndexFromType<T>();
    }

    TypePtr GetRawType() const { return type; }
    void* GetData() { return data.get(); }
    const void* GetData() const { return data.get(); }

    template <typename T>
    T& Get(size_t i)
    {
        NEPTOOLS_ASSERT_MSG(Is<T>(i), "specified item is not T");
        return *reinterpret_cast<T*>(&data[type->items[i].offset]);
    }

    template <typename T>
    const T& Get(size_t i) const
    {
        NEPTOOLS_ASSERT_MSG(Is<T>(i), "specified item is not T");
        return *reinterpret_cast<const T*>(&data[type->items[i].offset]);
    }

    template <typename Ret = void, typename... FunArgs>
    Ret Visit(size_t i, FunArgs&&... f)
    { return VisitHlp<Ret, 0, Args...>(this, i, std::forward<FunArgs>(f)...); }

    template <typename... FunArgs>
    void ForEach(FunArgs&&... f)
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit(i, std::forward<FunArgs>(f)...);
    }

    // const version
    template <typename Ret = void, typename... FunArgs>
    Ret Visit(size_t i, FunArgs&&... f) const
    { return VisitHlp<Ret, 0, Args...>(this, i, std::forward<FunArgs>(f)...); }

    template <typename... FunArgs>
    void ForEach(FunArgs&&... args) const
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit(i, std::forward<FunArgs>(args)...);
    }

private:
    TypePtr type;
    std::unique_ptr<char[]> data;
};

template <typename... Args>
constexpr const size_t DynamicStruct<Args...>::SIZE_OF[];

template <typename... Args>
constexpr const size_t DynamicStruct<Args...>::ALIGN_OF[];

}
#endif
