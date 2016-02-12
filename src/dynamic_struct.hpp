#ifndef DYNAMIC_STRUCT_HPP
#define DYNAMIC_STRUCT_HPP
#pragma once

#include <cstdint>
#include <memory>
#include <typeindex>
#include <vector>
#include <boost/intrusive_ptr.hpp>

namespace DynamicStruct
{

struct TypeDescription
{
    std::type_index type;
    size_t size, align;

    void (*default_construct)(void* ptr);
    //void (*assign)(void* ptr, const void* other);
    void (*destruct)(void* ptr);
};

template <typename T> struct TypeDB
{
    static const TypeDescription entry;
};

template <typename T> const TypeDescription TypeDB<T>::entry{
    typeid(T), sizeof(T), alignof(T),
    [](void* ptr)
    {
        new (ptr) T;
    },
    /*
    [](void* ptr, const void* other)
    {
        *static_cast<T*>(ptr) = *static_cast<const T*>(other);
    },
    */
    [](void* ptr)
    {
        static_cast<T*>(ptr)->~T();
    }};

struct StructType;
class StructTypeBuilder
{
public:
    auto& GetDesc() { return desc; }
    const auto& GetDesc() const { return desc; }

    template <typename T>
    void Add() { desc.push_back(&TypeDB<T>::entry); }

    boost::intrusive_ptr<const StructType> Build();
private:
    std::vector<const TypeDescription*> desc;
};

struct StructType
{
    StructType(const StructType&) = delete;
    void operator=(const StructType&) = delete;
    ~StructType() = delete;

    size_t refcount;
    size_t item_count, size;
    struct Item
    {
        const TypeDescription* desc;
        size_t offset;
    };
    Item items[1];

    template <typename Ret, typename T, typename... Rest, typename Fun>
    Ret Visit(size_t i, Fun&& f) const
    {
        if (items[i].desc->type == typeid(T))
            return f(static_cast<T*>(nullptr));
        else
            return Visit<Ret, Rest...>(i, std::forward<Fun>(f));
    }

    template <typename Ret, typename Fun>
    Ret Visit(size_t, Fun&&) const
    {
        throw std::runtime_error{"Struct::Visit: invalid type"};
    }

    template <typename... Args, typename Fun>
    void ForEach(Fun&& f) const
    {
        for (size_t i = 0; i < item_count; ++i)
            Visit<void, Args...>(i, std::forward<Fun>(f));
    }
};

using StructTypePtr = boost::intrusive_ptr<const StructType>;

inline void intrusive_ptr_add_ref(const StructType* desc)
{ ++const_cast<StructType*>(desc)->refcount; }
inline void intrusive_ptr_release(const StructType* desc)
{
    if (--const_cast<StructType*>(desc)->refcount == 0)
        ::operator delete(const_cast<StructType*>(desc));
}

class Struct
{
public:
    Struct() = default;
    Struct(const StructTypePtr& type);
    Struct(Struct&& o) : type{o.type}, data{std::move(o.data)}
    { o.type = nullptr; o.data = nullptr; }
    Struct& operator=(Struct&& o);
    ~Struct();

    size_t GetSize() const { return type->item_count; }
    std::type_index GetType(size_t i) const
    {
        BOOST_ASSERT(i < GetSize());
        return type->items[i].desc->type;
    }

    StructTypePtr GetRawType() const { return type; }
    void* GetData() { return data.get(); }
    const void* GetData() const { return data.get(); }

    template <typename T>
    T& Get(size_t i)
    {
        BOOST_ASSERT(GetType(i) == typeid(T));
        return *reinterpret_cast<T*>(&data[type->items[i].offset]);
    }

    template <typename T>
    const T& Get(size_t i) const
    {
        BOOST_ASSERT(GetType(i) == typeid(T));
        return *reinterpret_cast<const T*>(&data[type->items[i].offset]);
    }

    template <typename Ret, typename T, typename... Rest, typename Fun>
    Ret Visit(size_t i, Fun&& f)
    {
        if (GetType(i) == typeid(T))
            return f(Get<T>(i));
        else
            return Visit<Ret, Rest...>(i, std::forward<Fun>(f));
    }

    template <typename Ret, typename Fun>
    Ret Visit(size_t, Fun&&)
    {
        throw std::runtime_error{"Struct::Visit: invalid type"};
    }

    template <typename... Args, typename Fun>
    void ForEach(Fun&& f)
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit<void, Args...>(i, std::forward<Fun>(f));
    }

    // const version
    template <typename Ret, typename T, typename... Rest, typename Fun>
    Ret Visit(size_t i, Fun&& f) const
    {
        if (GetType(i) == typeid(T))
            return f(Get<T>(i));
        else
            return Visit<Ret, Rest...>(i, std::forward<Fun>(f));
    }

    template <typename Ret, typename Fun>
    Ret Visit(size_t, Fun&&) const
    {
        throw std::runtime_error{"Struct::Visit: invalid type"};
    }

    template <typename... Args, typename Fun>
    void ForEach(Fun&& f) const
    {
        for (size_t i = 0; i < type->item_count; ++i)
            Visit<void, Args...>(i, std::forward<Fun>(f));
    }

private:
    StructTypePtr type;
    std::unique_ptr<char[]> data;
};

}

#endif
