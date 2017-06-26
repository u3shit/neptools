#ifndef UUID_9372D768_26F4_4386_A5FB_FD14564CA6A3
#define UUID_9372D768_26F4_4386_A5FB_FD14564CA6A3
#pragma once

#include "intrusive.hpp"
#include "../assert.hpp"
#include "../check.hpp"
#include "../meta.hpp"
#include "../shared_ptr.hpp"
#include "../lua/type_traits.hpp"
#include "../lua/dynamic_object.hpp"

#include <boost/intrusive/circular_list_algorithms.hpp>
#include <boost/intrusive/pointer_traits.hpp>
#include <iterator>

namespace Neptools
{

struct ParentListNode
{
    ParentListNode* prev;
    ParentListNode* next;
    ParentListNode* parent = nullptr;
};

struct ParentListNodeTraits
{
    using node = ParentListNode;
    using node_ptr = node*;
    using const_node_ptr = const node*;

    static node_ptr get_previous(const_node_ptr n) noexcept { return n->prev; }
    static void set_previous(node_ptr n, node_ptr v) noexcept { n->prev = v; }
    static node_ptr get_next(const_node_ptr n) noexcept { return n->next; }
    static void set_next(node_ptr n, node_ptr v) noexcept { n->next = v; }
    static node_ptr get_parent(const_node_ptr n) noexcept { return n->parent; }
    static void set_parent(node_ptr n, node_ptr v) noexcept { n->parent = v; }
};

class ParentListHook : private ParentListNode
{
public:
    ParentListHook() = default;
    ParentListHook(const ParentListHook&) = delete;
    void operator=(const ParentListHook&) = delete;

    ~ParentListHook() noexcept
    {
        NEPTOOLS_ASSERT_MSG(!is_linked(), "destroying linked node");
    }

    bool is_root() const noexcept { return parent == this; }
    bool is_linked() const noexcept { return parent; }

    void unlink() noexcept
    {
        NEPTOOLS_ASSERT(is_linked());
        NEPTOOLS_ASSERT(!is_root());
        next->prev = prev;
        prev->next = next;
        parent = nullptr;
    }

    template <typename T, typename Tag> friend struct ParentListBaseHookTraits;
};

template <typename Tag = struct DefaultTag>
struct ParentListBaseHook : ParentListHook {};

template <typename T, typename Tag = DefaultTag>
struct ParentListBaseHookTraits
{
    using Hook = ParentListBaseHook<Tag>;
    static_assert(std::is_base_of_v<Hook, T>,
                  "T must inherit from ParentListBaseHook<Tag>");

    using node_traits = ParentListNodeTraits;
    using value_type = T;
    using node_ptr = node_traits::node_ptr;
    using const_node_ptr = node_traits::const_node_ptr;
    using pointer = T*;
    using const_pointer = const T*;

    static node_ptr to_node_ptr(value_type& v) noexcept
    { return static_cast<Hook*>(&v); }
    static const_node_ptr to_node_ptr(const value_type& v) noexcept
    { return static_cast<const Hook*>(&v); }
    static pointer to_value_ptr(node_ptr n) noexcept
    { return static_cast<pointer>(static_cast<Hook*>(n)); }
    static const_pointer to_value_ptr(const_node_ptr n) noexcept
    { return static_cast<const_pointer>(static_cast<const Hook*>(n)); }
};

template <typename Traits, bool IsConst>
class ParentListIterator
    : public std::iterator<std::bidirectional_iterator_tag,
                           typename Traits::value_type>
{
    using NodePtr = typename Traits::node_ptr;
    using ConstNodePtr = typename Traits::const_node_ptr;
    using NodeTraits = typename Traits::node_traits;

    ParentListIterator(ConstNodePtr ptr) noexcept
        : ptr{const_cast<NodePtr>(ptr)} {}
public:
    using RawT = typename Traits::value_type;
    using T = std::conditional_t<IsConst, const RawT, RawT>;

    ParentListIterator() = default;
    ParentListIterator(const ParentListIterator<Traits, false>& o) noexcept
        : ptr{o.ptr} {}
    ParentListIterator(T& val) noexcept
        : ptr{const_cast<NodePtr>(Traits::to_node_ptr(val))} {}

    ParentListIterator& operator=(const ParentListIterator<Traits, false>& o) noexcept
    {
        ptr = o.ptr;
        return *this;
    }

    T& operator*() const noexcept { return *Traits::to_value_ptr(ptr); }
    T* operator->() const noexcept { return Traits::to_value_ptr(ptr); }

    bool operator==(const ParentListIterator& o) const noexcept
    { return ptr == o.ptr; }
    bool operator!=(const ParentListIterator& o) const noexcept
    { return ptr != o.ptr; }

#define NEPTOOLS_GEN(op, fun)                       \
    ParentListIterator& operator op() noexcept      \
    {                                               \
        ptr = NodeTraits::fun(ptr);                 \
        return *this;                               \
    }                                               \
    ParentListIterator operator op(int) noexcept    \
    {                                               \
        auto ret = *this;                           \
        ptr = NodeTraits::fun(ptr);                 \
        return ret;                                 \
    }
    NEPTOOLS_GEN(++, get_next) NEPTOOLS_GEN(--, get_previous)
#undef NEPTOOLS_GEN

private:
    NodePtr ptr = nullptr;

    template <typename, bool> friend class ParentListIterator;
    template <typename, typename, typename> friend class ParentList;
};

struct NullTraits {};

template <typename T, typename LifetimeTraits = NullTraits,
          typename Traits = ParentListBaseHookTraits<T>>
class ParentList : public Lua::SmartObject, private Traits::node_traits::node
{
    NEPTOOLS_LUA_CLASS;
public:
    using value_traits = Traits;
    using pointer = typename value_traits::pointer;
    using const_pointer = typename value_traits::const_pointer;
    using value_type = typename boost::intrusive::pointer_traits<pointer>::element_type;
    using reference = typename boost::intrusive::pointer_traits<pointer>::reference;
    using const_reference = typename boost::intrusive::pointer_traits<const_pointer>::reference;
    using difference_type = typename boost::intrusive::pointer_traits<pointer>::difference_type;
    using size_type = std::size_t;
    using iterator = ParentListIterator<Traits, false>;
    using const_iterator = ParentListIterator<Traits, true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_traits = typename Traits::node_traits;
    using node = typename node_traits::node;
    using node_ptr = typename node_traits::node_ptr;
    using const_node_ptr = typename node_traits::const_node_ptr;
private:
    using ListAlgo = boost::intrusive::circular_list_algorithms<node_traits>;
public:

    // O(1)
    ParentList() noexcept { Init(); }
    template <typename Iterator>
    NEPTOOLS_NOLUA ParentList(Iterator b, Iterator e)
    {
        Init();
        insert(end(), b, e);
    }

    // warning! O(n.size())
    NEPTOOLS_NOLUA ParentList(ParentList&& o) noexcept
    {
        for (auto n = node_traits::get_next(o.GetRoot()); n != o.GetRoot();
             n = node_traits::get_next(n))
            node_traits::set_parent(n, GetRoot());
        Init();
        ListAlgo::swap_nodes(GetRoot(), o.GetRoot());
    }

    ParentList& operator=(ParentList&& o) noexcept
    {
        clear();
        swap(o);
        return *this;
    }

    // O(size())
    ~ParentList() noexcept { clear(); }

    // warning! O(n)
    void swap(ParentList& o) noexcept
    {
        ListAlgo::swap_nodes(GetRoot(), o.GetRoot());

        for (auto it = node_traits::get_next(GetRoot()); it != GetRoot();
             it = node_traits::get_next(it))
            node_traits::set_parent(it, GetRoot());

        for (auto it = node_traits::get_next(o.GetRoot()); it != o.GetRoot();
             it = node_traits::get_next(it))
            node_traits::set_parent(it, o.GetRoot());
    }

    // push*, pop*, front, back, *begin, *end -> O(1)
#define NEPTOOLS_GEN(name, link_dir, get_dir)                       \
    template <typename Checker = Check::Assert>                     \
    void push_##name(reference ref) noexcept(Checker::IS_NOEXCEPT)  \
    {                                                               \
        auto node = Traits::to_node_ptr(ref);                       \
        CheckUnlinked<Checker>(node);                               \
        ListAlgo::link_dir(GetRoot(), node);                        \
        NodeAdded(node);                                            \
    }                                                               \
    template <typename Checker = Check::Assert>                     \
    void pop_##name() noexcept(Checker::IS_NOEXCEPT)                \
    {                                                               \
        NEPTOOLS_CHECK(std::out_of_range, !empty(),                 \
                       "ParentList::pop_" #name);                   \
        auto node = node_traits::get_dir(GetRoot());                \
        ListAlgo::unlink(node);                                     \
        NodeRemoved(node);                                          \
    }
    NEPTOOLS_GEN(back, link_before, get_previous)
    NEPTOOLS_GEN(front, link_after, get_next)
#undef NEPTOOLS_GEN

#define NEPTOOLS_GEN(ret, name, opt_const, fun)                             \
    template <typename Checker = Check::Assert>                             \
    ret name() opt_const noexcept(Checker::IS_NOEXCEPT)                     \
    {                                                                       \
        NEPTOOLS_CHECK(std::out_of_range, !empty(), "ParentList::" #name);  \
        return *Traits::to_value_ptr(node_traits::fun(GetRoot()));          \
    }
    NEPTOOLS_GEN(reference, front, , get_next)
    NEPTOOLS_GEN(NEPTOOLS_NOLUA const_reference, front, const, get_next)
    NEPTOOLS_GEN(reference, back, , get_previous)
    NEPTOOLS_GEN(NEPTOOLS_NOLUA const_reference, back, const, get_previous)
#undef NEPTOOLS_GEN

#define NEPTOOLS_GEN(ret, name, opt_const, node)        \
    NEPTOOLS_NOLUA ret name() opt_const noexcept { return ret{node}; }
#define NEPTOOLS_GEN2(ret, const_ret, name, node)   \
    NEPTOOLS_GEN(ret, name, , node)                 \
    NEPTOOLS_GEN(const_ret, name, const, node)      \
    NEPTOOLS_GEN(const_ret, c##name, const, node)
#define NEPTOOLS_GEN3(name, rname, node)                                    \
    NEPTOOLS_GEN2(iterator, const_iterator, name, node)                     \
    NEPTOOLS_GEN2(reverse_iterator, const_reverse_iterator, rname, node)

    NEPTOOLS_GEN3(begin, rend, node_traits::get_next(GetRoot()))
    NEPTOOLS_GEN3(end, rbegin, GetRoot())
#undef NEPTOOLS_GEN3
#undef NEPTOOLS_GEN2
#undef NEPTOOLS_GEN

    // O(size())
    size_type size() const noexcept { return ListAlgo::count(GetRoot())-1; }
    // O(1)
    bool empty() const noexcept { return ListAlgo::unique(GetRoot()); }

    // O(n) both
    void shift_backwards(size_type n = 1) noexcept
    { ListAlgo::move_forward(GetRoot(), n); }
    void shift_forward(size_type n = 1) noexcept
    { ListAlgo::move_backwards(GetRoot(), n); }

    // O(1)
    template <typename Checker = Check::Assert>
    iterator erase(const_iterator it) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtr<Checker>(it.ptr);
        auto ret = it; ++ret;
        ListAlgo::unlink(it.ptr);
        NodeRemoved(it.ptr);
        return ret.ptr;
    }

    // O(distance(b, e))
    template <typename Checker = Check::Assert>
    iterator erase(const_iterator b, const_iterator e) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtr<Checker>(b.ptr);
        CheckNodePtrEnd<Checker>(e.ptr);
        if constexpr (!Checker::IS_NOP)
            for (auto it = b; it != e; ++it)
                NEPTOOLS_CHECK(ItemNotInContainer, it.ptr != GetRoot(),
                               "Invalid range");

        for (auto it = b; it != e; ++it)
            NodeRemoved(it.ptr);

        ListAlgo::unlink(b.ptr, e.ptr);
        return e.ptr;
    }

    // O(size())
    void clear() noexcept
    {
        for (auto it = begin(); it != end(); ++it)
            NodeRemoved(it.ptr);
        ListAlgo::init_header(GetRoot());
    }

    // clone_from?

    // O(1)
    template <typename Checker = Check::Assert>
    iterator insert(const_iterator p, reference ref) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(p.ptr);

        auto node = Traits::to_node_ptr(ref);
        CheckUnlinked<Checker>(node);
        ListAlgo::link_before(p.ptr, node);
        NodeAdded(node);
        return node;
    }

    // O(distance(b, e))
    template <typename Checker = Check::Assert, typename Iterator>
    NEPTOOLS_NOLUA void insert(const_iterator p, Iterator b, Iterator e)
        noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(p.ptr);
        if constexpr (!Checker::IS_NOP)
            for (auto it = b; it != e; ++it)
                CheckUnlinked<Checker>(Traits::to_node_ptr(*it));

        for (auto it = b; it != e; ++it)
        {
            auto node = Traits::to_node_ptr(*it);
            ListAlgo::link_before(p.ptr, node);
            NodeAdded(node);
        }
    }

    // O(distance(b, e))
    template <typename Checker = Check::Assert, typename Iterator>
    NEPTOOLS_NOLUA void assign(Iterator b, Iterator e)
        noexcept(Checker::IS_NOEXCEPT)
    {
        auto old_last = --end();
        insert<Checker>(end(), b, e); // may throw
        erase<std::conditional_t<
            std::is_same_v<Checker, Check::No>, Check::No, Check::Assert>>(
                begin(), ++old_last);
    }

    // O(x.size())
    template <typename Checker = Check::Assert>
    void splice(const_iterator p, ParentList& x) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(p.ptr);

        for (auto it = x.begin(); it != x.end(); ++it)
            node_traits::set_parent(it.ptr, GetRoot());
        ListAlgo::transfer(p.ptr, x.begin().ptr, x.end().ptr);
    }
    // O(1)
    template <typename Checker = Check::Assert>
    void splice(const_iterator p, ParentList& x, const_iterator new_ele)
        noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(p.ptr);
        x.CheckNodePtr<Checker>(new_ele.ptr);

        node_traits::set_parent(new_ele.ptr, GetRoot());
        ListAlgo::transfer(p.ptr, new_ele.ptr);
    }
    // O(distance(b, e))
    template <typename Checker = Check::Assert>
    void splice(const_iterator p, ParentList& x, const_iterator b,
                const_iterator e) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(p.ptr);
        x.CheckNodePtr<Checker>(b.ptr);
        x.CheckNodePtrEnd<Checker>(e.ptr);

        for (auto it = b; it != e; ++it)
            node_traits::set_parent(it.ptr, GetRoot());
        ListAlgo::transfer(p.ptr, b.ptr, e.ptr);
    }

    // O(n log n), n=size(); exception->basic guarantee
    NEPTOOLS_LUAGEN(hidden=not cls.alias.comparable)
    void sort() { sort(std::less<value_type>{}); }
    template <typename Predicate>
    NEPTOOLS_LUAGEN(template_params={"::Neptools::Lua::FunctionWrapGen<bool>"})
    void sort(Predicate cmp)
    {
        // based on
        // http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html
        for (size_t k = 1; ; k *= 2)
        {
            // break cycle
            node_traits::set_next(node_traits::get_previous(GetRoot()), nullptr);
            auto p = node_traits::get_next(GetRoot());
            ListAlgo::init_header(GetRoot()); // GetRoot(): L in paper
            size_t merges = 0;

            while (p)
            {
                ++merges;
                auto q = p;

                size_t psize;
                for (psize = 0; q && psize < k; ++psize)
                    q = node_traits::get_next(q);

                size_t qsize = k;
                try
                {
                    while (psize > 0 || (qsize > 0 && q))
                    {
                        node_ptr e;
                        if (psize == 0 || (qsize > 0 && q && cmp(
                            *Traits::to_value_ptr(q), *Traits::to_value_ptr(p))))
                        {
                            e = q;
                            q = node_traits::get_next(q);
                            --qsize;
                        }
                        else
                        {
                            e = p;
                            p = node_traits::get_next(p);
                            --psize;
                        }
                        ListAlgo::link_before(GetRoot(), e);
                    }
                }
                catch (...)
                {
                    // uh-oh. Basic guarantee, make sure no node is lost...
                    for (; psize > 0; --psize)
                    {
                        auto e = p;
                        p = node_traits::get_next(p);
                        ListAlgo::link_before(GetRoot(), e);
                    }
                    for (; q; )
                    {
                        auto e = q;
                        q = node_traits::get_next(q);
                        ListAlgo::link_before(GetRoot(), e);
                    }
                    throw;
                }
                p = q;
            }

            if (merges <= 1) return;
        }
    }

    // O(size() + o.size()); exception->basic guarantee
    template <typename Checker = Check::Assert>
    NEPTOOLS_LUAGEN(hidden=not cls.alias.comparable)
    void merge(ParentList& o) { merge<Checker>(o, std::less<value_type>{}); }
    template <typename Checker = Check::Assert, typename Predicate>
    NEPTOOLS_LUAGEN(template_params={
        "::Neptools::Check::Throw","::Neptools::Lua::FunctionWrapGen<bool>"})
    void merge(ParentList& o, Predicate cmp)
    {
        NEPTOOLS_CHECK(ContainerConsistency, &o != this,
                       "Trying to merge ParentList with itself");
        auto tit = begin(), oit = o.begin();
        auto tend = end(), oend = o.end();
        while (oit != oend)
        {
            if (tit == tend || cmp(*oit, *tit)) splice(tit, o, oit++);
            else ++tit;
        }
        NEPTOOLS_ASSERT(o.empty());
    }

    // O(size())
    void reverse() noexcept { ListAlgo::reverse(GetRoot()); }

    // O(size()); exception->basic guarantee
    NEPTOOLS_LUAGEN(hidden=not cls.alias.comparable)
    void remove(const_reference val)
    { remove_if([&val](auto& x) { return x == val; }); }
    template <typename Predicate>
    NEPTOOLS_LUAGEN(template_params={"::Neptools::Lua::FunctionWrapGen<bool>"})
    void remove_if(Predicate p)
    {
        for (auto it = begin(); it != end();)
            if (p(*it)) it = erase(it);
            else ++it;
    }

    // O(size()); exception->basic guarantee
    NEPTOOLS_LUAGEN(hidden=not cls.alias.comparable)
    void unique() { unique(std::equal_to<value_type>{}); }
    template <typename Predicate>
    NEPTOOLS_LUAGEN(template_params={"::Neptools::Lua::FunctionWrapGen<bool>"})
    void unique(Predicate p)
    {
        auto it = begin();
        auto next = it; ++next;
        NEPTOOLS_ASSERT((next == end()) == (size() <= 1));

        while (next != end())
            if (p(*it, *next)) next = erase(next);
            else it = next++;
    }

    // O(1)
    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA iterator iterator_to(reference ref)
        noexcept(Checker::IS_NOEXCEPT)
    {
        node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedThis<Checker>(node);
        return node;
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA const_iterator iterator_to(const_reference ref) const
        noexcept(Checker::IS_NOEXCEPT)
    {
        const_node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedThis<Checker>(node);
        return node;
    }

    // static funs
    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static ParentList& container_from_iterator(iterator it)
        noexcept(Checker::IS_NOEXCEPT)
    {
        CheckLinkedAny<Checker>(it.ptr);
        return *static_cast<ParentList*>(node_traits::get_parent(it.ptr));
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static const ParentList& container_from_iterator(const_iterator it)
        noexcept(Checker::IS_NOEXCEPT)
    {
        CheckLinkedAny<Checker>(it.ptr);
        return *static_cast<const ParentList*>(node_traits::get_parent(it.ptr));
    }


    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static iterator s_iterator_to(reference ref)
        noexcept(Checker::IS_NOEXCEPT)
    {
        node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedAny<Checker>(node);
        return node;
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static const_iterator s_iterator_to(const_reference ref)
        noexcept(Checker::IS_NOEXCEPT)
    {
        const_node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedAny<Checker>(node);
        return node;
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static ParentList& get_parent(reference ref)
        noexcept(Checker::IS_NOEXCEPT)
    {
        node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedAny<Checker>(node);
        return *static_cast<ParentList*>(node_traits::get_parent(node));
    }

    template <typename Checker = Check::Assert>
    NEPTOOLS_NOLUA static const ParentList& get_parent(const_reference ref)
        noexcept(Checker::IS_NOEXCEPT)
    {
        const_node_ptr node = Traits::to_node_ptr(ref);
        CheckLinkedAny<Checker>(node);
        return *static_cast<const ParentList*>(node_traits::get_parent(node));
    }

    NEPTOOLS_NOLUA static ParentList* opt_get_parent(reference ref) noexcept
    { return static_cast<ParentList*>(node_traits::get_parent(Traits::to_node_ptr(ref))); }
    NEPTOOLS_NOLUA static const ParentList* opt_get_parent(const_reference ref) noexcept
    { return static_cast<ParentList*>(node_traits::get_parent(Traits::to_node_ptr(ref))); }
private:
    void Init() noexcept
    {
        ListAlgo::init_header(GetRoot());
        node_traits::set_parent(GetRoot(), GetRoot());
    }

    template <typename U, typename = void> struct HasAdd : std::false_type {};
    template <typename U> struct HasAdd<U, std::void_t<decltype(U::add)>>
        : std::true_type {};

    template <typename U, typename = void> struct HasRemove : std::false_type {};
    template <typename U> struct HasRemove<U, std::void_t<decltype(U::remove)>>
        : std::true_type {};

    void NodeAdded(node_ptr nd) noexcept
    {
        node_traits::set_parent(nd, GetRoot());
        if constexpr (HasAdd<LifetimeTraits>::value)
        {
            static_assert(
                noexcept(LifetimeTraits::add(*this, std::declval<reference>())),
                "LifetimeTraits::add must be noexcept");
            LifetimeTraits::add(*this, *Traits::to_value_ptr(nd));
        }
    }

    void NodeRemoved(node_ptr nd) noexcept
    {
        node_traits::set_parent(nd, nullptr);
        if constexpr (HasRemove<LifetimeTraits>::value)
        {
            static_assert(
                noexcept(LifetimeTraits::remove(*this, std::declval<reference>())),
                "LifetimeTraits::remove must be noexcept");
            LifetimeTraits::remove(*this, *Traits::to_value_ptr(nd));
        }
    }

    template <typename Checker>
    void CheckNodePtr(const_node_ptr ptr) noexcept(Checker::IS_NOEXCEPT)
    {
        CheckNodePtrEnd<Checker>(ptr);
        NEPTOOLS_CHECK(
            ItemNotInContainer, ptr != GetRoot(), "Item is the root item");
    }

    template <typename Checker>
    void CheckNodePtrEnd(const_node_ptr ptr) noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(
            ItemNotInContainer, node_traits::get_parent(ptr) == GetRoot(),
            "Item not in this list");
    }

    template <typename Checker>
    static void CheckUnlinked(const_node_ptr ptr) noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(
            ItemAlreadyAdded, node_traits::get_parent(ptr) == nullptr,
            "Item already linked");
    }

    template <typename Checker>
    static void CheckLinkedAny(const_node_ptr ptr) noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(
            ItemNotInContainer, node_traits::get_parent(ptr),
            "Item not in any container");
    }

    template <typename Checker>
    void CheckLinkedThis(const_node_ptr ptr) const noexcept(Checker::IS_NOEXCEPT)
    {
        NEPTOOLS_CHECK(
            ItemNotInContainer, node_traits::get_parent(ptr) == GetRoot(),
            "Item not in this container");
    }

    node_ptr GetRoot() noexcept { return this; }
    const_node_ptr GetRoot() const noexcept { return this; }
};

template <typename T, typename A, typename B>
inline void swap(ParentList<T,A,B>& a, ParentList<T,A,B>& b) noexcept
{ a.swap(b); }

}

#endif
