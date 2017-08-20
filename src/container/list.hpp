#ifndef UUID_D0B9414F_339D_4016_9096_25DCC9CAE2F6
#define UUID_D0B9414F_339D_4016_9096_25DCC9CAE2F6
#pragma once

#include "../utils.hpp"
#include <boost/intrusive/list.hpp>

namespace Neptools
{

  template <typename Item, typename Traits, typename... Args>
  class List : private boost::intrusive::list<Item, Args...>, private Traits
  {
    using list = boost::intrusive::list<Item, Args...>;
    static constexpr bool NOEXCEPT_ADD = noexcept(std::declval<Traits>().add(
      std::declval<List&>(), std::declval<typename list::value_type&>()));

    static constexpr bool NOEXCEPT_REMOVE = noexcept(std::declval<Traits>().remove(
      std::declval<List&>(), std::declval<typename list::value_type&>()));
    static_assert(NOEXCEPT_REMOVE);

    static constexpr bool NOEXCEPT_MOVE = !Traits::is_movable || noexcept(
      Traits::move(std::declval<List&>(), std::declval<List&>(),
                   std::declval<typename list::value_type&>()));
    static_assert(NOEXCEPT_MOVE);

  public:
    // types
    using typename list::value_traits;
    using typename list::pointer;
    using typename list::const_pointer;
    using typename list::value_type;
    using typename list::reference;
    using typename list::const_reference;
    using typename list::difference_type;
    using typename list::size_type;
    using typename list::iterator;
    using typename list::const_iterator;
    using typename list::reverse_iterator;
    using typename list::const_reverse_iterator;
    using typename list::node_traits;
    using typename list::node;
    using typename list::node_ptr;
    using typename list::const_node_ptr;
    using typename list::node_algorithms;
    using typename list::header_holder_type;

    List() = default;
    explicit List(const value_traits& vt) : list{vt} {}
    explicit List(Traits ts) : Traits{std::move(ts)} {}
    explicit List(const value_traits& vt, Traits ts)
      : list{vt}, Traits{std::move(ts)} {}

    template <typename Iterator>
    List(Iterator b, Iterator e, const value_traits& vt = {}) : list{vt}
    { insert(cend(), b, e); }
    ~List() noexcept { clear(); }

    void push_back(reference it) noexcept(NOEXCEPT_ADD)
    {
      Traits::add(*this, it);
      list::push_back(it);
    }

    void push_front(reference it) noexcept(NOEXCEPT_ADD)
    {
      Traits::add(*this, it);
      list::push_front(it);
    }

    void pop_back() noexcept { list::pop_back_and_dispose(Disposer{this}); }
    void pop_front() noexcept { list::pop_front_and_dispose(Disposer{this}); }

    using list::front;
    using list::back;
    using list::begin;
    using list::cbegin;
    using list::end;
    using list::cend;
    using list::rbegin;
    using list::crbegin;
    using list::rend;
    using list::crend;
    using list::size;
    using list::empty;
    using list::swap;
    using list::shift_backwards;
    using list::shift_forward;

    iterator erase(const_iterator it) noexcept
    { return list::erase_and_dispose(it, Disposer{this}); }
    iterator erase(const_iterator b, const_iterator e) noexcept
    { return list::erase_and_dispose(b, e, Disposer{this}); }

    void clear() noexcept { list::clear_and_dispose(Disposer{this}); }

    // clone_from?

    iterator insert(const_iterator it, reference val) noexcept(NOEXCEPT_ADD)
    { Traits::add(*this, val); return list::insert(it, val); }
    template <typename Iterator>
    // add throws -> basic guarantee only
    void insert(const_iterator it, Iterator b, Iterator e) noexcept(NOEXCEPT_ADD)
    { for (; b != e; ++b) insert(it, *b); }
    template <typename Iterator>
    void assign(Iterator b, Iterator e) noexcept(NOEXCEPT_ADD)
    { clear(); insert(cend(), b, e); }

#define NEPTOOLS_MOVABLE_ONLY(...)                              \
    template <__VA_ARGS__ typename T,                           \
              std::enable_if_t<std::is_same<T, Item>::value &&  \
                               Traits::is_movable, bool> = true>

    NEPTOOLS_MOVABLE_ONLY() void splice(const_iterator p, List& l) noexcept
    {
      for (auto& el : l) Traits::move(l, *this, l);
      list::splice(p, l);
    }

    NEPTOOLS_MOVABLE_ONLY()
    void splice(const_iterator p, List& l, const_iterator el) noexcept
    {
      Traits::move(l, *this, *el);
      list::splice(p, l, el);
    }

    NEPTOOLS_MOVABLE_ONLY()
    void splice(
      const_iterator p, List& l, const_iterator b, const_iterator e) noexcept
    {
      size_t dist = 0;
      for (auto it = b; it != e; ++it)
      {
        ++dist;
        Traits::move(l, *this, *it);
      }
      list::splice(p, l, b, e, dist);
    }

    using list::sort;

    NEPTOOLS_MOVABLE_ONLY()
    void merge(List& l) noexcept { merge(l, std::less<value_type>());}
    NEPTOOLS_MOVABLE_ONLY(typename Predicate,)
    void merge(List& l, Predicate p) noexcept
    {
      for (auto& it : l) Traits::move(l, *this, it);
      list::merge(l, p);
    }
#undef NEPTOOLS_MOVABLE_ONLY

    using list::reverse;

    void remove(const_reference it)
    { list::remove_and_dispose(it, Disposer{this}); }
    template <typename Pred> void remove_if(Pred p)
    { list::remove_and_dispose_if(p, Disposer{this}); }

    // requires operator==
    void unique() { list::unique_and_dispose(Disposer{}); }
    template <typename Pred> void unique(Pred p)
    { list::unique_and_dispose(p, Disposer{this}); }

    using list::iterator_to;
    using list::check;

    // static funs
    static List& container_from_end_iterator(iterator it) noexcept
    { return static_cast<List&>(list::container_from_end_iterator(it)); }
    static const List& container_from_end_iterator(const_iterator it) noexcept
    { return static_cast<const List&>(list::container_from_end_iterator(it)); }

    using list::s_iterator_to;

    // consts
    using list::constant_time_size;
    using list::stateful_value_traits;
    using list::has_container_from_iterator;

  private:
    struct Disposer
    {
      List* list;
      void operator()(pointer p) { list->Traits::remove(*list, *p); }
    };
};

}

#endif
