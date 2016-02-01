#ifndef CL3_FILE_HPP
#define CL3_FILE_HPP
#pragma once

#include "../buffer.hpp"
#include "../context.hpp"

namespace Cl3
{

class HeaderItem;
class FileCollectionItem;

class File : public Context
{
public:
    File();
    File(std::shared_ptr<Buffer> buf) : File{buf, 0, buf->GetSize()} {}
    File(std::shared_ptr<Buffer> buf, size_t offset, size_t len);
    File(const std::string& fname) : File{ReadFile(fname)} {}
    File(const char* fname) : File{ReadFile(fname)} {}

    void Fixup() override;

    const HeaderItem& GetHeader() const noexcept;
    HeaderItem& GetHeader() noexcept;

    const FileCollectionItem& GetFileCollection() const;
    FileCollectionItem& GetFileCollection();
};

Item* PadItem(Item* item);

}
#endif
