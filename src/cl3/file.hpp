#ifndef CL3_FILE_HPP
#define CL3_FILE_HPP
#pragma once

#include "../buffer.hpp"
#include "../context.hpp"

namespace Cl3
{

class File final : public Context
{
public:
    File(std::shared_ptr<Buffer> buf);
    File(const std::string& fname) : File{ReadFile(fname)} {}
    File(const char* fname) : File{ReadFile(fname)} {}
    File(const File&) = delete;
    void operator=(const File&) = delete;
};

}
#endif
