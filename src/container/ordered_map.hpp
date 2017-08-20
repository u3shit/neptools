#ifndef UUID_6D8552D7_1BF6_468A_A6E3_D0829B5390C1
#define UUID_6D8552D7_1BF6_468A_A6E3_D0829B5390C1
#pragma once

#include "intrusive.hpp"

#include <libshit/check.hpp>
#include <libshit/meta.hpp>
#include <libshit/shared_ptr.hpp>
#include <libshit/lua/dynamic_object.hpp>
#include <libshit/lua/type_traits.hpp>

#include <boost/intrusive/set.hpp>
#include <vector>

namespace Neptools
{

  template <typename T, typename KeyOfValue,
            typename Compare = std::less<typename KeyOfValue::type>>
  class OrderedMap;

  using OrderedMapItemHook = boost::intrusive::set_base_hook<
    boost::intrusive::tag<struct OrderedMapItemTag>,
    boost::intrusive::optimize_size<true>,
    LinkMode>;

  struct OrderedMapItem : public Libshit::RefCounted, public OrderedMapItemHook
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
    OrderedMapIterator(
      const OrderedMapIterator<Smart, std::remove_const_t<T>>& it)
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
      std::ptrdiff_t n, OrderedMapIterator it) noexcept             \
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
  class LIBSHIT_LUAGEN(post_register="\
    luaL_getmetatable(bld, \"neptools_ipairs\");\
    bld.SetField(\"__ipairs\");\
") OrderedMap : public Libshit::Lua::SmartObject
  {
    LIBSHIT_LUA_CLASS;
  private:
    static_assert(std::is_base_of_v<OrderedMapItem, T>);
    using ElemType = Libshit::NotNull<Libshit::SmartPtr<T>>;
    using VectorType = std::vector<ElemType>;
    using VectorPtr = typename VectorType::pointer;
    using ConstVectorPtr = typename VectorType::const_pointer;
    using SetType = boost::intrusive::set<
      T, boost::intrusive::base_hook<OrderedMapItemHook>,
      boost::intrusive::constant_time_size<false>,
      boost::intrusive::compare<Compare>,
      boost::intrusive::key_of_value<Traits>>;

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = OrderedMapIterator<ElemType, T>;
    using const_iterator = OrderedMapIterator<ElemType, const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::size_t;
    using size_type = std::size_t;
    using key_type = typename SetType::key_type;

    OrderedMap() = default;
    LIBSHIT_NOLUA OrderedMap(OrderedMap&&) = default;
    ~OrderedMap() noexcept { for (auto& x : vect) RemoveItem(*x); }
    // set should have a move ctor but no copy ctor

    T& at(size_t i)
    {
      LIBSHIT_ASSERT(VectorIndex(*vect.at(i)) == i);
      return *vect.at(i);
    }
    LIBSHIT_NOLUA const T& at(size_t i) const
    {
      LIBSHIT_ASSERT(VectorIndex(*vect.at(i)) == i);
      return *vect.at(i);
    }

    T& operator[](size_t i) noexcept
    {
      LIBSHIT_ASSERT(i < size() && VectorIndex(*vect[i]) == i);
      return *vect[i];
    }
    const T& operator[](size_t i) const noexcept
    {
      LIBSHIT_ASSERT(i < size() && VectorIndex(*vect[i]) == i);
      return *vect[i];
    }

#define NEPTOOLS_GEN(name, pre, post, val)                              \
    template <typename Checker = Libshit::Check::Assert>                \
      pre T& name() post noexcept(Checker::IS_NOEXCEPT)                 \
    {                                                                   \
      LIBSHIT_CHECK(std::out_of_range, !empty(), "OrderedMap::" #name); \
      LIBSHIT_ASSERT(VectorIndex(*vect.name()) == (val));               \
      return *vect.name();                                              \
    }
#define NEPTOOLS_GEN2(name, val) \
    NEPTOOLS_GEN(name, , , val)  \
    NEPTOOLS_GEN(name, LIBSHIT_NOLUA const, const, val)

    NEPTOOLS_GEN2(front, 0)
    NEPTOOLS_GEN2(back, vect.size()-1)
#undef NEPTOOLS_GEN2
#undef NEPTOOLS_GEN

#define NEPTOOLS_GEN(dir, typ)                                  \
    LIBSHIT_NOLUA typ##iterator dir() noexcept                  \
    { return typ##iterator{ToPtr(vect.dir())}; }                \
    LIBSHIT_NOLUA const_##typ##iterator c##dir() const noexcept \
    { return const_##typ##iterator{ToPtr(vect.c##dir())}; }     \
    LIBSHIT_NOLUA const_##typ##iterator dir() const noexcept    \
    { return const_##typ##iterator{ToPtr(vect.c##dir())}; }

    NEPTOOLS_GEN(begin,) NEPTOOLS_GEN(end,)
    NEPTOOLS_GEN(rbegin, reverse_) NEPTOOLS_GEN(rend, reverse_)
#undef NEPTOOLS_GEN

    bool empty() const noexcept
    {
      LIBSHIT_ASSERT(vect.empty() == set.empty());
      return vect.empty();
    }
    LIBSHIT_LUAGEN() LIBSHIT_LUAGEN(name="__len") // alias as # operator
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

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA std::pair<iterator, bool> insert(
      const_iterator p, const ElemType& t)
    {
      CheckPtrEnd<Checker>(ToPtr(p));
      return InsertGen<Checker>(p, t);
    }
    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA std::pair<iterator, bool> insert(
      const_iterator p, ElemType&& t)
    {
      CheckPtrEnd<Checker>(ToPtr(p));
      return InsertGen<Checker>(p, std::move(t));
    }

    template <typename Checker = Libshit::Check::Assert, typename... Args>
    LIBSHIT_NOLUA std::pair<iterator, bool>
    emplace(const_iterator p, Args&&... args)
    {
      CheckPtrEnd<Checker>(ToPtr(p));
      return InsertGen<Checker>(
        p, Libshit::MakeSmart<T>(std::forward<Args>(args)...));
    }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA iterator erase(const_iterator it) noexcept
    {
      auto ptr = ToPtr(it);
      CheckPtr<Checker>(ptr);
      set.erase(ToSetIt(ptr));
      return VectErase(ptr);
    }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA iterator erase(const_iterator b, const_iterator e)
      noexcept(Checker::IS_NOEXCEPT)
    {
      auto bptr = ToPtr(b), eptr = ToPtr(e);
      CheckPtrEnd<Checker>(bptr); CheckPtrEnd<Checker>(eptr);
      LIBSHIT_CHECK(ItemNotInContainer, bptr <= eptr, "Invalid range");

      auto bi = ToVectIt(bptr), ei = ToVectIt(eptr);
      for (auto it = b; it != e; ++it)
      {
        RemoveItem(const_cast<T&>(*it));
        set.erase(ToSetIt(ToPtr(it)));
      }
      auto ret = vect.erase(bi, ei);
      FixupIndex(ret);
      return iterator{ToPtr(ret)};
    }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA std::pair<iterator, bool> push_back(const ElemType& t)
    { return InsertGen<Checker>(end(), t); }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA std::pair<iterator, bool> push_back(ElemType&& t)
    { return InsertGen<Checker>(end(), std::move(t)); }

    template <typename Checker = Libshit::Check::Assert, typename... Args>
    LIBSHIT_NOLUA std::pair<iterator, bool> emplace_back(Args&&... args)
    {
      return InsertGen<Checker>(
        end(), Libshit::MakeSmart<T>(std::forward<Args>(args)...));
    }

    template <typename Checker = Libshit::Check::Assert>
    void pop_back() noexcept
    {
      LIBSHIT_CHECK(std::out_of_range, !empty(), "pop_back");
      auto& back = *vect.back();
      RemoveItem(back);
      set.erase(Traits{}(back));
      vect.pop_back();
    }

    void swap(OrderedMap& o)
    {
      vect.swap(o.vect);
      set.swap(o.set);
    }

    // boost extensions
    // not template checker -> we need 2 different checks...
    LIBSHIT_NOLUA iterator nth(size_t i) noexcept
    { return iterator{&vect[i]}; }
    LIBSHIT_NOLUA iterator checked_nth(size_t i)
    {
      if (i < size()) return nth(i);
      else LIBSHIT_THROW(std::out_of_range{"OrderedMap::checked_nth"});
    }
    LIBSHIT_NOLUA iterator checked_nth_end(size_t i)
    {
      if (i <= size()) return nth(i);
      else LIBSHIT_THROW(std::out_of_range{"OrderedMap::checked_nth_end"});
    }

    LIBSHIT_NOLUA const_iterator nth(size_t i) const noexcept
    { return const_iterator{&vect[i]}; }
    LIBSHIT_NOLUA const_iterator checked_nth(size_t i) const
    {
      if (i < size()) return nth(i);
      else LIBSHIT_THROW(std::out_of_range{"OrderedMap::checked_nth"});
    }
    LIBSHIT_NOLUA const_iterator checked_nth_end(size_t i) const
    {
      if (i <= size()) return nth(i);
      else LIBSHIT_THROW(std::out_of_range{"OrderedMap::checked_nth_end"});
    }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA size_t index_of(const_iterator it)
      const noexcept(Checker::IS_NOEXCEPT)
    {
      if (it == end()) return size();

      CheckPtr<Checker>(ToPtr(it));
      return VectorIndex(**ToPtr(it));
    }

    template <typename Checker = Libshit::Check::Assert>
    size_t index_of(const T& t) const noexcept(Checker::IS_NOEXCEPT)
    {
      CheckPtr<Checker>(ToPtr(t));
      return VectorIndex(**ToPtr(t));
    }

    // map portions
    size_t count(const key_type& key) const { return set.count(key); }
    // comp must yield the same ordering as the used comparator in the tree...
    template <typename Key, typename Comp>
    LIBSHIT_NOLUA size_t count(const Key& key, Comp comp) const
    { return set.count(key, comp); }

    LIBSHIT_NOLUA iterator find(const key_type& key)
    { return ToMaybeEndIt(set.find(key)); }
    template <typename Key, typename Comp>
    LIBSHIT_NOLUA iterator find(const Key& key, Comp comp)
    { return ToMaybeEndIt(set.find(key, comp)); }

    LIBSHIT_NOLUA const_iterator find(const key_type& key) const
    { return ToMaybeEndIt(set.find(key)); }
    template <typename Key, typename Comp>
    LIBSHIT_NOLUA const_iterator find(const Key& key, Comp comp) const
    { return ToMaybeEndIt(set.find(key, comp)); }

    // misc intrusive
    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA iterator iterator_to(T& t) noexcept
    { return iterator{ToPtr(t)}; }
    LIBSHIT_NOLUA const_iterator iterator_to(const T& t) const noexcept
    { return const_iterator{ToPtr(t)}; }

    // return end() on invalid ptr
    LIBSHIT_NOLUA iterator checked_iterator_to(T& t) noexcept
    {
      if (VectorIndex(t) < size() && vect[VectorIndex(t)].get() == &t)
        return iterator{ToPtr(t)};
      else
        return end();
    }
    LIBSHIT_NOLUA const_iterator checked_iterator_to(const T& t) const noexcept
    {
      if (VectorIndex(t) < size() & vect[VectorIndex(t)].get() == &t)
        return const_iterator{ToPtr(t)};
      else
        return cend();
    }

    // we'd need pointer to smartptr inside vector
    // static iterator s_iterator_to(T& t) noexcept { return iterator{t}; }
    // static const_iterator s_iterator_to(const T& t) noexcept
    // { return const_iterator{t}; }

    template <typename Checker = Libshit::Check::Assert>
    LIBSHIT_NOLUA std::pair<iterator, bool> key_change(iterator it) noexcept
    {
      CheckPtr<Checker>(ToPtr(it));
      set.erase(ToSetIt(ToPtr(it)));
      auto ins = set.insert(*it);
      auto rit = it;
      if (!ins.second)
      {
        VectErase(ToPtr(it));
        rit = iterator{ToPtr(*ins.first)};
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

    template <typename Checker, typename U>
    std::pair<iterator, bool> InsertGen(const_iterator p, U&& t)
    {
      LIBSHIT_CHECK(ItemAlreadyAdded, VectorIndex(*t) == size_t(-1),
                    "Item alread added to an OrderedMap");

      typename SetType::insert_commit_data data{};
      auto itp = set.insert_check(Traits{}(*t), data);
      if (itp.second)
      {
        auto& ref = *t;
        auto it = vect.insert(ToVectIt(ToPtr(p)), std::forward<U>(t));
        // noexcept from here
        FixupIndex(it);
        set.insert_commit(ref, data);
        return {iterator{ToPtr(it)}, true};
      }

      return {iterator{ToPtr(*itp.first)}, false};
    }

    iterator VectErase(ConstVectorPtr ptr) noexcept
    {
      auto vit = ToVectIt(ptr); // prevent assert after RemoveItem
      RemoveItem(**ptr);
      auto ret = vect.erase(vit);
      FixupIndex(ret);
      return iterator{ToPtr(ret)};
    }

    template <typename Checker>
    void CheckPtr(ConstVectorPtr ptr) const noexcept(Checker::IS_NOEXCEPT)
    {
      LIBSHIT_CHECK(
        ItemNotInContainer, ptr >= &vect.front() && ptr <= &vect.back(),
        "Item not in this OrderedMap");
      LIBSHIT_ASSERT(VectorIndex(**ptr) < size() &&
                     &vect[VectorIndex(**ptr)] == ptr);
    }

    template <typename Checker>
    void CheckPtrEnd(ConstVectorPtr ptr) const noexcept(Checker::IS_NOEXCEPT)
    {
      LIBSHIT_CHECK(
        ItemNotInContainer, ptr >= &*vect.begin() && ptr <= &*vect.end(),
        "Item not in this OrderedMap");
      LIBSHIT_ASSERT(
        ptr == &*vect.end() ||
        (VectorIndex(**ptr) < size() && &vect[VectorIndex(**ptr)] == ptr));
    }

    ConstVectorPtr ToPtr(const OrderedMapItem& it) const noexcept
    { return &vect[VectorIndex(it)]; }
    ConstVectorPtr ToPtr(typename VectorType::const_iterator it) const noexcept
    { return &*it; }
    ConstVectorPtr ToPtr(typename SetType::const_iterator it) const noexcept
    { return &vect[VectorIndex(*it)]; }
    ConstVectorPtr ToPtr(const_iterator it) const noexcept { return it.ptr; }

    typename VectorType::iterator ToVectIt(ConstVectorPtr ptr) noexcept
    { return vect.begin() + (ptr - &*vect.begin()); }
    typename SetType::iterator ToSetIt(ConstVectorPtr ptr) noexcept
    { return set.iterator_to(**ptr); }

    iterator ToMaybeEndIt(typename SetType::const_iterator it) const
    {
      if (it == set.end())
        return iterator{&*vect.end()};
      else return iterator{ToPtr(it)};
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
