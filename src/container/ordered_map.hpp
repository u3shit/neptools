#ifndef UUID_6D8552D7_1BF6_468A_A6E3_D0829B5390C1
#define UUID_6D8552D7_1BF6_468A_A6E3_D0829B5390C1
#pragma once

#include "intrusive.hpp"
#include "../shared_ptr.hpp"
#include "../lua/dynamic_object.hpp"

#include <boost/intrusive/set.hpp>
#include <vector>

namespace Neptools
{

NEPTOOLS_GEN_EXCEPTION_TYPE(ItemAlreadyAdded, std::logic_error);

template <typename T, typename KeyOfValue,
          typename Compare = std::less<typename KeyOfValue::type>>
class OrderedMap;

using OrderedMapItemHook = boost::intrusive::set_base_hook<
    boost::intrusive::tag<struct OrderedMapItemTag>,
    boost::intrusive::optimize_size<true>,
    LinkMode>;

struct OrderedMapItem : public RefCounted, public OrderedMapItemHook
{
private:
    size_t vector_index = -1;
    template <typename T, typename KeyOfValue, typename Compare>
    friend class OrderedMap;
};

template <typename Smart, typename T>
class OrderedMapIterator
    : public std::iterator<std::random_access_iterator_tag, T>
{
    using VectorType = std::vector<Smart>;
    using Ptr = typename VectorType::pointer;
public:
    OrderedMapIterator() = default;
    friend class OrderedMapIterator<Smart, const T>;
    OrderedMapIterator(const OrderedMapIterator<Smart, std::remove_const_t<T>>& it)
        : ptr{it.ptr} {}

    T& operator*() const noexcept { return **ptr; }
    T* operator->() const noexcept { return &**ptr; }
    T& operator[](std::ptrdiff_t n) { return *(*this + n); }

#define NEPTOOLS_GEN(op)                                         \
    bool operator op(const OrderedMapIterator& o) const noexcept \
    { return ptr op o.ptr; }
    NEPTOOLS_GEN(==) NEPTOOLS_GEN(!=) NEPTOOLS_GEN(<) NEPTOOLS_GEN(<=)
    NEPTOOLS_GEN(>) NEPTOOLS_GEN(>=)
#undef NEPTOOLS_GEN

#define NEPTOOLS_GEN(op)                                            \
    OrderedMapIterator& operator op##op() noexcept                  \
    { op##op ptr; return *this; }                                   \
    OrderedMapIterator operator op##op(int) noexcept                \
    { auto ret = *this; op##op ptr; return ret; }                   \
                                                                    \
    OrderedMapIterator& operator op##=(std::ptrdiff_t n) noexcept   \
    { ptr op##= n; return *this; }                                  \
    OrderedMapIterator operator op(std::ptrdiff_t n) const noexcept \
    { auto ret = *this; ret op##= n; return ret; }                  \
    friend OrderedMapIterator operator op(                          \
        std::ptrdiff_t n, OrderedMapIterator it) noexcept           \
    { it op##= n; return it; }

    NEPTOOLS_GEN(+) NEPTOOLS_GEN(-)
#undef NEPTOOLS_GEN
    std::ptrdiff_t operator-(OrderedMapIterator o) const noexcept
    { return ptr - o.ptr; }

private:
    template <typename U>
    explicit OrderedMapIterator(U ptr) : ptr{const_cast<Ptr>(ptr)} {}
    Ptr ptr = nullptr;

    std::remove_const_t<T>* Get() const noexcept { return ptr->get(); }

    template <typename U, typename KeyOfValue, typename Compare>
    friend class OrderedMap;
};

template <typename T, typename Traits, typename Compare>
class NEPTOOLS_LUAGEN(post_register="\
  luaL_getmetatable(vm, \"neptools_ipairs\");\
  bld.SetField(\"__ipairs\");\
") OrderedMap : public Lua::SmartObject
{
    NEPTOOLS_LUA_CLASS;
private:
    using VectorType = std::vector<NotNull<SmartPtr<T>>>;
    using SetType = boost::intrusive::set<
        T, boost::intrusive::base_hook<OrderedMapItemHook>,
        boost::intrusive::constant_time_size<false>,
        boost::intrusive::compare<Compare>,
        boost::intrusive::key_of_value<Traits>>;

public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = OrderedMapIterator<NotNull<SmartPtr<T>>, T>;
    using const_iterator = OrderedMapIterator<NotNull<SmartPtr<T>>, const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::size_t;
    using size_type = std::size_t;
    using key_type = typename SetType::key_type;

    OrderedMap() = default;
    ~OrderedMap() { for (auto& x : vect) RemoveItem(*x); }
    // set should have a move ctor but no copy ctor

    T& at(size_t i)
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.at(i)) == i);
        return *vect.at(i);
    }
    NEPTOOLS_NOLUA const T& at(size_t i) const
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.at(i)) == i);
        return *vect.at(i);
    }

    T& operator[](size_t i)
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.at(i)) == i);
        return *vect[i];
    }
    const T& operator[](size_t i) const
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.at(i)) == i);
        return *vect[i];
    }

    NEPTOOLS_NOLUA T& front()
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.front()) == 0);
        return *vect.front();
    }
    NEPTOOLS_NOLUA const T& front() const
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.front()) == 0);
        return *vect.front();
    }
    NEPTOOLS_NOLUA T& back()
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.back()) == vect.size()-1);
        return *vect.back();
    }
    NEPTOOLS_NOLUA const T& back() const
    {
        NEPTOOLS_ASSERT(VectorIndex(*vect.back()) == vect.size()-1);
        return *vect.back();
    }

#define NEPTOOLS_GEN(dir, typ)                                      \
    NEPTOOLS_NOLUA typ##iterator dir() noexcept                     \
    { return ToIt(vect.dir()); }                                    \
    NEPTOOLS_NOLUA const_##typ##iterator c##dir() const noexcept    \
    { return ToIt(vect.c##dir()); }                                 \
    NEPTOOLS_NOLUA const_##typ##iterator dir() const noexcept       \
    { return ToIt(vect.c##dir()); }
    NEPTOOLS_GEN(begin,) NEPTOOLS_GEN(end,)
    NEPTOOLS_GEN(rbegin, reverse_) NEPTOOLS_GEN(rend, reverse_)
#undef NEPTOOLS_GEN

    bool empty() const noexcept
    {
        NEPTOOLS_ASSERT(vect.empty() == set.empty());
        return vect.empty();
    }
    NEPTOOLS_LUAGEN() NEPTOOLS_LUAGEN(name="__len") // alias as # operator
    size_t size() const noexcept { return vect.size(); }
    // boost::intrusive doesn't have max size
    size_t max_size() const noexcept { return vect.max_size(); }

    // only modifies the vector part
    void reserve(size_t cap) { vect.reserve(cap); }
    size_t capacity() const noexcept { return vect.capacity(); }
    void shrink_to_fit() { vect.shrink_to_fit(); }

    // modify both
    void clear() noexcept
    {
        for (auto& x : vect) RemoveItem(*x);
        set.clear(); vect.clear();
    }

    NEPTOOLS_NOLUA std::pair<iterator, bool> insert(
        const_iterator p, const NotNull<SmartPtr<T>>& t)
    { return InsertGen(p, t); }
    NEPTOOLS_NOLUA std::pair<iterator, bool> insert(
        const_iterator p, NotNull<SmartPtr<T>>&& t)
    { return InsertGen(p, std::move(t)); }

    template <typename... Args>
    NEPTOOLS_NOLUA std::pair<iterator, bool>
    emplace(const_iterator p, Args&&... args)
    { return InsertGen(p, MakeSmart<T>(std::forward<Args>(args)...)); }

    NEPTOOLS_NOLUA iterator erase(const_iterator it) noexcept
    {
        set.erase(ToSetIt(it));
        return VectErase(it);
    }

    NEPTOOLS_NOLUA iterator erase(const_iterator b, const_iterator e) noexcept
    {
        // RemoveItem would break assert in ToVecIt if it'd executed at vect.erase
        auto bi = ToVectIt(b), ei = ToVectIt(e);
        for (auto it = b; it != e; ++it)
        {
            RemoveItem(const_cast<T&>(*it));
            set.erase(ToSetIt(it));
        }
        auto ret = vect.erase(bi, ei);
        FixupIndex(ret);
        return ToIt(ret);
    }

    NEPTOOLS_NOLUA std::pair<iterator, bool> push_back(
        const NotNull<SmartPtr<T>>& t)
    { return InsertGen(end(), t); }
    NEPTOOLS_NOLUA std::pair<iterator, bool> push_back(NotNull<SmartPtr<T>>&& t)
    { return InsertGen(end(), std::move(t)); }
    template <typename... Args>
    NEPTOOLS_NOLUA std::pair<iterator, bool> emplace_back(Args&&... args)
    { return InsertGen(end(), MakeSmart<T>(std::forward<Args>(args)...)); }

    NEPTOOLS_NOLUA void pop_back() noexcept
    {
        auto& back = *vect.back();
        RemoveItem(back);
        set.erase(Traits{}(back));
        vect.pop_back();
    }
    NEPTOOLS_LUAGEN(name="pop_back") void checked_pop_back() noexcept
    { if (!empty()) pop_back(); }

    void swap(OrderedMap& o)
    {
        vect.swap(o.vect);
        set.swap(o.set);
    }

    // boost extensions
    NEPTOOLS_NOLUA iterator nth(size_t i) noexcept
    { return ToIt(vect.begin() + i); }
    NEPTOOLS_NOLUA iterator checked_nth(size_t i)
    {
        if (i < size()) return nth(i);
        else NEPTOOLS_THROW(std::out_of_range{"OrderedMap::checked_nth"});
    }
    NEPTOOLS_NOLUA iterator checked_nth_end(size_t i)
    {
        if (i <= size()) return nth(i);
        else NEPTOOLS_THROW(std::out_of_range{"OrderedMap::checked_nth_end"});
    }

    NEPTOOLS_NOLUA const_iterator nth(size_t i) const noexcept
    { return ToIt(vect.begin() + i); }
    NEPTOOLS_NOLUA const_iterator checked_nth(size_t i) const
    {
        if (i < size()) return nth(i);
        else NEPTOOLS_THROW(std::out_of_range{"OrderedMap::checked_nth"});
    }
    NEPTOOLS_NOLUA const_iterator checked_nth_end(size_t i) const
    {
        if (i <= size()) return nth(i);
        else NEPTOOLS_THROW(std::out_of_range{"OrderedMap::checked_nth_end"});
    }

    NEPTOOLS_NOLUA size_t index_of(const_iterator it) const noexcept
    { return VectorIndex(*it); }

    // map portions
    NEPTOOLS_LUAGEN(args={"const /*$= class */::key_type&"})
    size_t count(const key_type& key) const { return set.count(key); }
    // comp must yield the same ordering as the used comparator in the tree...
    template <typename Key, typename Comp>
    NEPTOOLS_NOLUA size_t count(const Key& key, Comp comp) const
    { return set.count(key, comp); }

    NEPTOOLS_NOLUA iterator find(const key_type& key)
    { return ToMaybeEndIt(set.find(key)); }
    template <typename Key, typename Comp>
    NEPTOOLS_NOLUA iterator find(const Key& key, Comp comp)
    { return ToMaybeEndIt(set.find(key, comp)); }

    NEPTOOLS_NOLUA const_iterator find(const key_type& key) const
    { return ToMaybeEndIt(set.find(key)); }
    template <typename Key, typename Comp>
    NEPTOOLS_NOLUA const_iterator find(const Key& key, Comp comp) const
    { return ToMaybeEndIt(set.find(key, comp)); }

    // misc intrusive
    NEPTOOLS_NOLUA iterator iterator_to(T& t) noexcept { return ToIt(t); }
    NEPTOOLS_NOLUA const_iterator iterator_to(const T& t) const noexcept
    { return ToIt(t); }

    // return end() on invalid ptr
    NEPTOOLS_NOLUA iterator checked_iterator_to(T& t) noexcept
    {
        if (vect[VectorIndex(t)].get() == &t) return ToIt(t);
        else return end();
    }
    NEPTOOLS_NOLUA const_iterator checked_iterator_to(const T& t) const noexcept
    {
        if (vect[VectorIndex(t)].get() == &t) return ToIt(t);
        else return cend();
    }

    // we'd need pointer to smartptr inside vector
    // static iterator s_iterator_to(T& t) noexcept { return iterator{t}; }
    // static const_iterator s_iterator_to(const T& t) noexcept
    // { return const_iterator{t}; }

    NEPTOOLS_NOLUA std::pair<iterator, bool> key_change(iterator it) noexcept
    {
        set.erase(ToSetIt(it));
        auto ins = set.insert(*it);
        auto rit = it;
        if (!ins.second)
        {
            VectErase(it);
            rit = ToIt(*ins.first);
        }
        return {rit, ins.second};
    }

private:
    static size_t& VectorIndex(OrderedMapItem& i) noexcept
    { return i.vector_index; }
    static size_t VectorIndex(const OrderedMapItem& i) noexcept
    { return i.vector_index; }

    void FixupIndex(typename VectorType::iterator b) noexcept
    { for (; b != vect.end(); ++b) VectorIndex(**b) = b - vect.begin(); }

    void RemoveItem(T& t) noexcept { VectorIndex(t) = -1; }

    template <typename U>
    std::pair<iterator, bool> InsertGen(const_iterator p, U&& t)
    {
        if (VectorIndex(*t) != size_t(-1))
            NEPTOOLS_THROW(ItemAlreadyAdded{"Item already added to an OrderedMap"});

        typename SetType::insert_commit_data data{};
        auto itp = set.insert_check(Traits{}(*t), data);
        if (itp.second)
        {
            auto& ref = *t;
            auto it = vect.insert(ToVectIt(p), std::forward<U>(t));
            // noexcept from here
            FixupIndex(it);
            set.insert_commit(ref, data);
            return {ToIt(it), true};
        }

        return {ToIt(*itp.first), false};
    }

    iterator VectErase(const_iterator it) noexcept
    {
        auto vit = ToVectIt(it); // prevent assert after RemoveItem
        RemoveItem(*it.Get());
        auto ret = vect.erase(vit);
        FixupIndex(ret);
        return ToIt(ret);
    }

    iterator ToIt(const OrderedMapItem& it) const noexcept
    {
        NEPTOOLS_ASSERT(vect[VectorIndex(it)].get() == &it);
        return iterator{&vect[VectorIndex(it)]};
    }
    iterator ToIt(typename VectorType::iterator it) noexcept
    {
        NEPTOOLS_ASSERT(it == vect.end() ||
                        VectorIndex(**it) == size_t(it-vect.begin()));
        return iterator{&*it};
    }
    const_iterator ToIt(typename VectorType::const_iterator it) const noexcept
    {
        NEPTOOLS_ASSERT(it == vect.end() ||
                        VectorIndex(**it) == size_t(it-vect.begin()));
        return const_iterator{&*it};
    }

    typename VectorType::iterator ToVectIt(iterator it)
    { return vect.begin() + (it-begin()); }
    typename VectorType::const_iterator ToVectIt(const_iterator it) const
    { return vect.begin() + (it-begin()); }
    typename SetType::const_iterator ToSetIt(const_iterator it) const
    {
        NEPTOOLS_ASSERT(it != cend());
        return set.iterator_to(*it);
    }

    iterator ToMaybeEndIt(typename SetType::const_iterator it) const
    {
        if (it == set.end())
            return iterator{&*vect.end()};
        else return ToIt(*it);
    }

    VectorType vect;
    SetType set;
};


template <typename T, typename Traits, typename Compare>
void swap(OrderedMap<T, Traits, Compare>& a,
          OrderedMap<T, Traits, Compare>& b)
{ a.swap(b); }

}

#endif
