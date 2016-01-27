#include "buffer.hpp"
#include <system_error>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MmappedBuffer::MmappedBuffer(const char* fname)
{
    int fd = open(fname, O_RDONLY);
    if (fd < 0)
        throw std::system_error{std::error_code{errno, std::system_category()}};

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        auto sav = errno;
        close(fd);
        throw std::system_error{std::error_code{sav, std::system_category()}};
    }

    len = sb.st_size;
    if (len == 0)
    {
        close(fd);
        throw std::runtime_error("0-sized file");
    }
    ptr = static_cast<Byte*>(mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0));
    auto sav = errno;
    close(fd);

    if (!ptr)
        throw std::system_error{std::error_code{sav, std::system_category()}};
}

MmappedBuffer::~MmappedBuffer()
{
    munmap(ptr, len);
}

StringBuffer::StringBuffer(std::string buf)
    : s{std::move(buf)}
{
    ptr = reinterpret_cast<Byte*>(&s[0]);
    len = s.size();
    read_only = false;
}

std::shared_ptr<Buffer> ReadFile(const char* fname)
{
    try
    {
        return std::make_shared<MmappedBuffer>(fname);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Failed to mmap file " << fname << ": " << e.what()
                  << std::endl;

        std::ifstream in;
        in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        in.open(fname, std::ifstream::binary);

        std::stringstream ss;
        ss << in.rdbuf();
        return std::make_shared<StringBuffer>(ss.str());
    }
}
