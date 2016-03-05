#ifndef UUID_310CE7F9_73D3_4140_BA43_9DA7CB4136D1
#define UUID_310CE7F9_73D3_4140_BA43_9DA7CB4136D1
#pragma once

#include <cstddef>
#include "../except.hpp"
#include "../utils.hpp"

extern Byte* image_base;

Byte* Find(const Byte* pattern, const Byte* mask, size_t length) noexcept;
inline Byte* FindOrDie(const Byte* pattern, const Byte* mask, size_t length)
{
    auto ret = Find(pattern, mask, length);
    if (!ret) THROW(std::runtime_error{"Couldn't find pattern"});
    return ret;
}

void* Hook(void* hook, void* dst, size_t copy);

template <typename T>
inline T Hook(void* hook, T dst, size_t copy)
{
    STATIC_ASSERT(sizeof(T) == 4);
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

#endif
