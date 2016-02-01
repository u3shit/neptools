#ifndef STCM_FILE_HPP
#define STCM_FILE_HPP
#pragma once

#include "../buffer.hpp"
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
