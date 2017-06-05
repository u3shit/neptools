#ifndef UUID_EA2FC01F_70AB_4D47_AB3B_D3EF94DA7AB5
#define UUID_EA2FC01F_70AB_4D47_AB3B_D3EF94DA7AB5
#pragma once

#include <type_traits>
#include <utility>

namespace Neptools
{

template <typename T>
constexpr std::size_t EmptySizeof = std::is_empty<T>::value ? 0 : sizeof(T);

template <typename X, template <char...> typename Wrap, typename Seq>
struct ToCharPack;
template <typename X, template <char...> typename Wrap, size_t... Idx>
struct ToCharPack<X, Wrap, std::index_sequence<Idx...>>
{
    using Type = Wrap<X::Get(Idx)...>;
};

template <typename X, template <char...> typename Wrap, typename Seq>
using ToCharPackV = typename ToCharPack<X, Wrap, Seq>::Type;

#define NEPTOOLS_LITERAL_CHARPACK(type, str) ([]{                              \
        struct X { static constexpr char Get(size_t i) { return (str)[i]; } }; \
        return ::Neptools::ToCharPackV<                                        \
            X, type, std::make_index_sequence<sizeof(str)-1>>{};               \
    }())


}

#endif
