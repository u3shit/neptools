#include "buffer.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

#if !defined(_WIN32) && !defined(_WIN64)
#include <system_error>
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
#endif

StringBuffer::StringBuffer(std::string buf)
    : s{std::move(buf)}
{
    ptr = reinterpret_cast<Byte*>(&s[0]);
    len = s.size();
    read_only = false;
}

std::shared_ptr<Buffer> ReadFile(const boost::filesystem::path& fname)
{
#if !defined(_WIN32) && !defined(_WIN64)
    try
    {
        return std::make_shared<MmappedBuffer>(fname.native());
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Failed to mmap file " << fname << ": " << e.what()
                  << std::endl;
#endif

        auto in = OpenIn(fname);

        std::stringstream ss;
        ss << in.rdbuf();
        return std::make_shared<StringBuffer>(ss.str());
#if !defined(_WIN32) && !defined(_WIN64)
    }
#endif
}
