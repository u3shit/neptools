#ifndef UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#define UUID_61A5519F_6624_43C0_8451_0BCA60B5D69A
#pragma once

#include "../../buffer.hpp"
#include "../context.hpp"

namespace Stcm
{

class File : public Context
{
public:
    File(std::shared_ptr<Buffer> buf) : File{buf, 0, buf->GetSize()} {}
    File(std::shared_ptr<Buffer> buf, size_t offset, size_t len);
    File(const std::string& fname) : File{ReadFile(fname)} {}
    File(const char* fname) : File{ReadFile(fname)} {}
};

}
#endif
