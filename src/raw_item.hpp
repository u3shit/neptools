#ifndef RAW_ITEM_HPP
#define RAW_ITEM_HPP
#pragma once

#include "item.hpp"

class RawItem final : public Item
{
public:
    RawItem(Key k, Context* ctx, std::shared_ptr<Buffer> buf) noexcept
        : Item{k, ctx}, offset{0}, len{buf->GetSize()}, buf{std::move(buf)} {}

    const Byte* GetPtr() const noexcept { return buf->GetPtr(); }
    size_t GetSize() const noexcept override { return len; }
    const Byte& operator[](size_t i) const noexcept
    { return buf->GetPtr()[offset + i]; }
    void Dump(std::ostream& os) const override;

    void Split(size_t pos, std::unique_ptr<Item> nitem);

protected:
    RawItem(Context* ctx, std::shared_ptr<Buffer> buf, size_t offset,
            size_t len, FilePosition position) noexcept
        : Item{{}, ctx, position}, offset{offset}, len{len}, buf{std::move(buf)} {}
private:
    size_t offset, len;
    std::shared_ptr<Buffer> buf;
};

#endif
