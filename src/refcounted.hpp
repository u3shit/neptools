#ifndef UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#define UUID_1472EFCC_6110_4A95_BDB7_2EE28E2207E4
#pragma once

#include <atomic>
#include <boost/intrusive_ptr.hpp>

namespace Neptools
{

class RefCounted
{
public:
    RefCounted() = default;
    RefCounted(const RefCounted&) = delete;
    void operator=(const RefCounted&) = delete;
    virtual ~RefCounted() = default;

    size_t use_count() const { return refcount; } // emulate boost refcount

private:
    mutable std::atomic<size_t> refcount{0};
    friend void intrusive_ptr_add_ref(const RefCounted* rc) noexcept
    { ++rc->refcount; }
    friend void intrusive_ptr_release(const RefCounted* rc) noexcept
    {
        if (--rc->refcount == 0) delete rc;
    }
};

template<typename T, typename... Args>
inline boost::intrusive_ptr<T> MakeRefCounted(Args&&... args)
{
    NEPTOOLS_STATIC_ASSERT(std::is_base_of<RefCounted, T>::value);
    return {new T(std::forward<Args>(args)...)};
}

}

#endif
