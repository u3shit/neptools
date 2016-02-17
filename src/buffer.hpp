#ifndef UUID_7E43A5C4_EF27_4794_AEF1_202C9776DD6D
#define UUID_7E43A5C4_EF27_4794_AEF1_202C9776DD6D
#pragma once

#include "utils.hpp"
#include <string>
#include <memory>

class Buffer
{
public:
    Buffer(const Buffer&) = delete;
    void operator=(const Buffer&) = delete;

    virtual ~Buffer() = default;

    const Byte* GetPtr() const noexcept { return ptr; }
    size_t GetSize() const noexcept { return len; }


protected:
    Buffer() = default;
    Buffer(Byte* ptr, size_t len, bool read_only)
        : ptr{ptr}, len{len}, read_only{read_only} {}

    Byte* ptr;
    size_t len;
    bool read_only;
};

#if !defined(_WIN32) && !defined(_WIN64)
class MmappedBuffer final : public Buffer
{
public:
    MmappedBuffer(const char* fname);
    MmappedBuffer(const std::string& fname) : MmappedBuffer(fname.c_str()) {}
    ~MmappedBuffer();
};
#endif

/*
class MemoryBuffer final : public Buffer
{
public:
    MemoryBuffer(Byte* data, size_t len) : Buffer{data, len, false} {}
    ~MemoryBuffer() { delete[] ptr; }
};
*/

class StringBuffer final : public Buffer
{
public:
    StringBuffer(std::string buf);
private:
    std::string s;
};

std::shared_ptr<Buffer> ReadFile(const fs::path& file);

#endif
