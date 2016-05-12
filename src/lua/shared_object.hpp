#ifndef UUID_D1053C4C_B56A_4475_8EAB_62319D91611F
#define UUID_D1053C4C_B56A_4475_8EAB_62319D91611F
#pragma once

#include "base.hpp"

namespace Neptools
{
namespace Lua
{

class SharedObject : public /* virtual? */ std::enable_shared_from_this<SharedObject>
{
public:
    static constexpr const char* TYPE_NAME = "object";

    SharedObject() = default;
    SharedObject(const SharedObject&) = delete;
    virtual ~SharedObject() = default;
    void operator=(const SharedObject&) = delete;

    static char TYPE_TAG;
    void PushLua(StateRef vm);

private:
    virtual void* GetTypeTag() const = 0;
};

template <typename Deriv, typename Base = SharedObject>
class SharedHelper : public Base
{
public:
    static_assert(std::is_base_of<SharedObject, Base>::value,
                  "invalid base class");
    static char TYPE_TAG;

private:
    using Base::Base;
    void* GetTypeTag() const override { return &TYPE_TAG; }
};

template <typename Deriv, typename Base>
char SharedHelper<Deriv, Base>::TYPE_TAG;

}
}

#endif
