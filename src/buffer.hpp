#ifndef BUFFER_HPP
#define BUFFER_HPP
#pragma once

#include <string>
#include <memory>

using Byte = unsigned char;

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

class MmappedBuffer final : public Buffer
{
public:
    MmappedBuffer(const char* fname);
    MmappedBuffer(const std::string& fname) : MmappedBuffer(fname.c_str()) {}
    ~MmappedBuffer();
};

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

std::shared_ptr<Buffer> ReadFile(const char* file);
inline std::shared_ptr<Buffer> ReadFile(const std::string& str)
{ return ReadFile(str.c_str()); }

#endif
