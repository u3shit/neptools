#ifndef UUID_6D829400_DD15_49CC_B55F_6F2565105828
#define UUID_6D829400_DD15_49CC_B55F_6F2565105828
#pragma once

#include <boost/filesystem/path.hpp>
#include <vector>

#include "../source.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define GEN_FWD(name, fld) \
    template <typename... Args> inline auto name(Args&&... args)  \
    { return (this->*fld)(std::forward<Args>(args)...); }

struct PakEntry
{
    unsigned field_000;
    unsigned file_index;
    char file_name[260];
    unsigned field_10c;
    size_t compressed_size;
    size_t uncompressed_size;
    unsigned is_compressed;
    unsigned field_11c;
};
STATIC_ASSERT(sizeof(PakEntry) == 0x120);

struct CpkHandlerFileInfo
{
    unsigned index;
    HANDLE handle;
    bool is_valid;
    PakEntry entry;
    unsigned data_start;
    unsigned read_pos;
    void* huffmann_hdr;
    void* block;
    int decoded_block_index;
};
STATIC_ASSERT(sizeof(CpkHandlerFileInfo) == 0x140);

struct CpkHandler
{
    unsigned vect0_begin, vect0_end, vect0_capacity;
    using FileInfo = CpkHandlerFileInfo;
    std::vector<FileInfo*> entry_vect;
    unsigned vect2_begin, vect2_end, vect2_capacity;
    char* data;
    char* hash_entries;
    char* names;
    char* basename;
    unsigned last_error;
    CRITICAL_SECTION crit_sec;

    char __thiscall OpenFile(const char* fname, size_t* out);
    char __thiscall CloseFile(unsigned index);
    char __thiscall Read(unsigned index, char* dst, size_t dst_size,
                         size_t* out_size_read);

    using OpenFilePtr = decltype(&CpkHandler::OpenFile);
    using CloseFilePtr = decltype(&CpkHandler::CloseFile);
    using ReadPtr = decltype(&CpkHandler::Read);

    static CpkHandler::OpenFilePtr orig_open_file;
    static CpkHandler::CloseFilePtr orig_close_file;
    static CpkHandler::ReadPtr orig_read;

    GEN_FWD(OrigOpenFile, orig_open_file);
    GEN_FWD(OrigCloseFile, orig_close_file);
    GEN_FWD(OrigRead, orig_read);

    static void Hook();

    Source GetSource(const char* fname);

private:
    FileInfo& GetEntryVect(size_t* out);
    bool OpenFsFile(
        const char* fname, const boost::filesystem::path& pth, size_t* out);
    bool OpenTxtFile(
        const char* fname, const boost::filesystem::path& pth, size_t* out);
};
STATIC_ASSERT(sizeof(CpkHandler) == 0x50);

struct CpkError : std::runtime_error, virtual boost::exception
{
    using std::runtime_error::runtime_error;
};
using CpkErrorCode = boost::error_info<struct CpkErrorCodeTag, int>;

#endif
