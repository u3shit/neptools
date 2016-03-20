#ifndef UUID_4C4561D8_78E4_438B_9804_61F42DB159F7
#define UUID_4C4561D8_78E4_438B_9804_61F42DB159F7
#pragma once

namespace Neptools
{

template <char... Args> struct StringContainer
{
    template <char C> using PushBack = StringContainer<Args..., C>;

    static constexpr const char str[sizeof...(Args)+1] = { Args... };
    static constexpr const wchar_t wstr[sizeof...(Args)+1] = { Args... };
};

template <char... Args>
constexpr const char StringContainer<Args...>::str[];

template <char... Args>
constexpr const wchar_t StringContainer<Args...>::wstr[];


namespace FileTools
{

// list helper
template <typename... Args> struct List;

template <typename In, typename Out> struct ListPopBack;
template <typename I, typename J, typename... InArgs, typename... OutArgs>
struct ListPopBack<List<I, J, InArgs...>, List<OutArgs...>>
{
    using Value = typename ListPopBack<
        List<J, InArgs...>, List<OutArgs..., I>>::Value;
};

template <typename I, typename Out>
struct ListPopBack<List<I>, Out>
{ using Value = Out; };

template <typename... Args> struct List
{
    template <typename T> using PushBack = List<Args..., T>;
    using PopBack = typename ListPopBack<List<Args...>, List<>>::Value;

    template <typename T> using PushFront = List<T, Args...>;
};

template <> struct List<>
{
    template <typename T> using PushBack = List<T>;
    template <typename T> using PushFront = List<T>;
};

// split path at /s
template <typename List, typename Building, char... Args> struct DoSplit;

template <typename List, typename Building, char... Rest>
struct DoSplit<List, Building, '/', Rest...>
{
    using Value = typename DoSplit<
        typename List::template PushBack<Building>, StringContainer<>, Rest...>::Value;
};

template <typename List, typename Building, char C, char... Rest>
struct DoSplit<List, Building, C, Rest...>
{
    using Value = typename DoSplit<
        List, typename Building::template PushBack<C>, Rest...>::Value;
};

template <typename List, typename Building>
struct DoSplit<List, Building>
{
    using Value = typename List::template PushBack<Building>;
};

template <char... Args>
using Split = typename DoSplit<List<>, StringContainer<>, Args...>::Value;

// normalize: remove . and .., strip src and everything before
template <typename In, typename Out> struct DoNormalize;

template <typename... InArgs, typename Out>
struct DoNormalize<List<StringContainer<'.'>, InArgs...>, Out>
{
    using Value = typename DoNormalize<List<InArgs...>, Out>::Value;
};

template <typename... InArgs, typename Out0, typename... OutRest>
struct DoNormalize<List<StringContainer<'.','.'>, InArgs...>, List<Out0, OutRest...>>
{
    using Out = List<Out0, OutRest...>;
    using Value = typename DoNormalize<List<InArgs...>, typename Out::PopBack>::Value;
};

template <typename... InArgs, typename Out>
struct DoNormalize<List<StringContainer<'s','r','c'>, InArgs...>, Out>
{
    using Value = typename DoNormalize<List<InArgs...>, List<>>::Value;
};

template <typename In0, typename... InRest, typename Out>
struct DoNormalize<List<In0, InRest...>, Out>
{
    using Value = typename DoNormalize<
        List<InRest...>, typename Out::template PushBack<In0>>::Value;
};

template <typename Out> struct DoNormalize<List<>, Out> { using Value = Out; };

template <typename In>
using Normalize = typename DoNormalize<In, List<>>::Value;


// join
template <typename In, typename Out> struct DoJoin;

template <char... Next, typename... Rest>
struct DoJoin<List<StringContainer<Next...>, Rest...>, StringContainer<>>
{
    using Value = typename DoJoin<
        List<Rest...>, StringContainer<Next...>>::Value;
};

template <char... Next, typename... Rest, char... Cur>
struct DoJoin<List<StringContainer<Next...>, Rest...>, StringContainer<Cur...>>
{
    using Value = typename DoJoin<
        List<Rest...>, StringContainer<Cur..., '/', Next...>>::Value;
};

template <typename Str> struct DoJoin<List<>, Str> { using Value = Str; };

template <typename List>
using Join = typename DoJoin<List, StringContainer<>>::Value;

// global helper
template <char... Args>
using GetFile = Join<Normalize<Split<Args...>>>;


template <typename Char, Char... Args>
constexpr auto operator"" _get_file() { return GetFile<Args...>::str; }

template <typename Char, Char... Args>
constexpr auto operator"" _get_wfile() { return GetFile<Args...>::wstr; }
}

}

#define NEPTOOLS_CONCAT2(a, b) a##b
#define NEPTOOLS_CONCAT(a, b) NEPTOOLS_CONCAT2(a, b)
#define NEPTOOLS_FILE ([]{ \
        using ::Neptools::FileTools::operator"" _get_file;  \
        return NEPTOOLS_CONCAT(__FILE__, _get_file);        \
    }())

#define NEPTOOLS_WFILE ([]{                                 \
        using ::Neptools::FileTools::operator"" _get_wfile; \
        return NEPTOOLS_CONCAT(__FILE__, _get_wfile);       \
    }())

#endif
