#ifndef UUID_310CE7F9_73D3_4140_BA43_9DA7CB4136D1
#define UUID_310CE7F9_73D3_4140_BA43_9DA7CB4136D1
#pragma once

#include <cstddef>
#include "../pattern.hpp"

namespace Neptools
{

extern Byte* image_base;
size_t GetImageSize() noexcept;

inline Byte* MaybeFindImage(const Pattern& pat) noexcept
{ return pat.MaybeFind(image_base, GetImageSize()); }
inline Byte* FindImage(const Pattern& pat)
{ return pat.Find(image_base, GetImageSize()); }

void* Hook(void* hook, void* dst, size_t copy);

template <typename T>
inline T Hook(void* hook, T dst, size_t copy)
{
    NEPTOOLS_STATIC_ASSERT(sizeof(T) == 4);
    union
    {
        T t;
        void* vd;
    } u;
    u.t = dst;
    u.vd = Hook(hook, u.vd, copy);
    return u.t;
}

template <typename T>
T& As(void* ptr) { return *static_cast<T*>(ptr); }

class Unprotect
{
public:
    Unprotect(void* ptr, size_t len);
    ~Unprotect();
    Unprotect(const Unprotect&) = delete;
    void operator=(const Unprotect&) = delete;

private:
    void* ptr;
    size_t len;
    unsigned long orig_prot;
};

}
#endif
