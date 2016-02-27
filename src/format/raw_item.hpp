#ifndef UUID_49BE292D_0E45_47B1_8901_97A22C0190F6
#define UUID_49BE292D_0E45_47B1_8901_97A22C0190F6
#pragma once

#include "context.hpp"
#include "../source.hpp"
#include <boost/assert.hpp>

class RawItem final : public Item
{
public:
    RawItem(Key k, Context* ctx, Source src, FilePosition pos = 0) noexcept
        : Item{k, ctx, pos}, src{std::move(src)} {}

    const Source& GetSource() const noexcept { return src; }
    uint64_t GetSize() const noexcept override { return src.GetSize(); }
    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;

    template <typename T>
    T* Split(size_t pos, std::unique_ptr<T> nitem)
    {
        T* ret = nitem.get();
        Split2(pos, std::move(nitem));
        return ret;
    }

    template <typename T, typename... Args>
    T* SplitCreate(size_t pos, Args&&... args)
    { return Split(pos, GetContext()->Create<T>(std::forward<Args>(args)...)); }

    RawItem* Split(size_t offset, size_t size);

    template <typename T>
    static auto Get(ItemPointer ptr)
    {
        auto& ritem = ptr.AsChecked<RawItem>();
        BOOST_ASSERT(ptr.offset <= ritem.GetSize());
        if (ptr.offset + sizeof(T) > ritem.GetSize())
            throw std::runtime_error{"Premature end of data"};

        struct Ret { RawItem& ritem; T t; };
        return Ret{
            std::ref(ritem),
            ritem.src.Pread<T>(ptr.offset)};
    }

    static auto GetSource(ItemPointer ptr, uint64_t len)
    {
        auto& ritem = ptr.AsChecked<RawItem>();
        BOOST_ASSERT(ptr.offset <= ritem.GetSize());
        if (len == uint64_t(-1)) len = ritem.GetSize() - ptr.offset;

        if (ptr.offset + len > ritem.GetSize())
            throw std::runtime_error{"Premature end of data"};
        struct Ret { RawItem& ritem; Source src; };
        return Ret{std::ref(ritem), {ritem.src, ptr.offset, len}};
    }

protected:
    std::unique_ptr<RawItem> InternalSlice(size_t offset, size_t size);
    void Split2(size_t pos, std::unique_ptr<Item> nitem);

private:
    DumpableSource src;
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

#endif
