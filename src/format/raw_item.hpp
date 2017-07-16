#ifndef UUID_49BE292D_0E45_47B1_8901_97A22C0190F6
#define UUID_49BE292D_0E45_47B1_8901_97A22C0190F6
#pragma once

#include "context.hpp"
#include "../source.hpp"
#include "../except.hpp"

namespace Neptools
{

class RawItem final : public Item
{
    NEPTOOLS_DYNAMIC_OBJECT;
public:
    RawItem(Key k, Context& ctx, Source src) noexcept
        : Item{k, ctx}, src{std::move(src)} {}
    NEPTOOLS_NOLUA
    RawItem(Key k, Context& ctx, Source src, FilePosition pos) noexcept
        : Item{k, ctx, pos}, src{std::move(src)} {}

    const Source& GetSource() const noexcept { return src; }
    FilePosition GetSize() const noexcept override { return src.GetSize(); }

    template <typename T>
    NEPTOOLS_LUAGEN(template_params={"::Neptools::Item"})
    T& Split(FilePosition pos, NotNull<RefCountedPtr<T>> nitem)
    {
        T& ret = *nitem;
        Split2(pos, std::move(nitem));
        return ret;
    }

    template <typename T, typename... Args>
    NEPTOOLS_NOLUA T& SplitCreate(FilePosition pos, Args&&... args)
    {
        auto ctx = GetContext();
        return Split(pos, ctx->Create<T>(std::forward<Args>(args)...));
    }

    RawItem& Split(FilePosition offset, FilePosition size);

    template <typename T>
    NEPTOOLS_NOLUA static auto Get(ItemPointer ptr)
    {
        auto& ritem = ptr.AsChecked<RawItem>();
        NEPTOOLS_ASSERT_MSG(ptr.offset <= ritem.GetSize(), "invalid offset");
        if (ptr.offset + sizeof(T) > ritem.GetSize())
            NEPTOOLS_THROW(DecodeError{"Premature end of data"});

        struct Ret { RawItem& ritem; T t; };
        return Ret{
            std::ref(ritem),
            ritem.src.PreadGen<T>(ptr.offset)};
    }

    struct GetSourceRet { RawItem& ritem; Source src; };
    static GetSourceRet GetSource(ItemPointer ptr, FilePosition len)
    {
        auto& ritem = ptr.AsChecked<RawItem>();
        NEPTOOLS_ASSERT_MSG(ptr.offset <= ritem.GetSize(), "invalid offset");
        if (len == FilePosition(-1)) len = ritem.GetSize() - ptr.offset;

        if (ptr.offset + len > ritem.GetSize())
            NEPTOOLS_THROW(DecodeError{"Premature end of data"});
        return {std::ref(ritem), {ritem.src, ptr.offset, len}};
    }

private:
    NotNull<RefCountedPtr<RawItem>> InternalSlice(
        FilePosition offset, FilePosition size);
    void Split2(FilePosition pos, NotNull<SmartPtr<Item>> nitem);

    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;

    Source src;
};

template<> struct Lua::TupleLike<RawItem::GetSourceRet>
{
    template <size_t I>
    static auto& Get(const RawItem::GetSourceRet& ret) noexcept
    {
        if constexpr (I == 0) return ret.ritem;
        else if constexpr (I == 1) return ret.src;
    }
    static constexpr size_t SIZE = 2;
};

template <typename T>
inline void MaybeCreate(ItemPointer ptr)
{
    auto item = ptr.Maybe<RawItem>();
    if (item)
        T::CreateAndInsert(ptr);
    else
        ptr.As0<T>(); // assert it
}

template <typename T>
inline void MaybeCreateUnchecked(ItemPointer ptr)
{
    if (ptr.Maybe<RawItem>())
        T::CreateAndInsert(ptr);
}


}
#endif
