#ifndef RAW_ITEM_HPP
#define RAW_ITEM_HPP
#pragma once

#include "item.hpp"
#include <boost/assert.hpp>

class RawItem final : public Item
{
public:
    RawItem(Key k, Context* ctx, std::shared_ptr<Buffer> buf) noexcept
        : Item{k, ctx}, offset{0}, len{buf->GetSize()}, buf{std::move(buf)} {}
    RawItem(Key k, Context* ctx, std::shared_ptr<Buffer> buf, size_t offset,
            size_t len, FilePosition position = 0) noexcept
        : Item{k, ctx, position}, offset{offset}, len{len}, buf{std::move(buf)}
    {
        BOOST_ASSERT(offset <= this->buf->GetSize() &&
                     offset + len <= this->buf->GetSize());
    }

    const Byte* GetPtr() const noexcept { return buf->GetPtr() + offset; }
    size_t GetSize() const noexcept override { return len; }
    const Byte& operator[](size_t i) const noexcept
    { return buf->GetPtr()[offset + i]; }
    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;

    template <typename T>
    T* Split(size_t pos, std::unique_ptr<T> nitem)
    {
        T* ret = nitem.get();
        Split2(pos, std::move(nitem));
        return ret;
    }

    RawItem* Split(size_t offset, size_t size);

    auto GetOffset() const noexcept { return offset; }
    auto GetBuffer() const noexcept { return buf; }

protected:
    std::unique_ptr<RawItem> InternalSlice(size_t offset, size_t size);
    void Split2(size_t pos, std::unique_ptr<Item> nitem);

private:
    size_t offset, len;
    std::shared_ptr<Buffer> buf;
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
