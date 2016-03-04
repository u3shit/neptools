#ifndef UUID_6D829400_DD15_49CC_B55F_6F2565105828
#define UUID_6D829400_DD15_49CC_B55F_6F2565105828
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

#include "../utils.hpp"

struct PakEntry
{
    unsigned field_000;
    unsigned file_index;
    char file_name[260];
    unsigned field_10c;
    unsigned compressed_size;
    unsigned uncompressed_size;
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

    char __thiscall OpenFile(const char* fname, int* out);
    char __thiscall CloseFile(unsigned index);
    char __thiscall Read(unsigned index, char* dst, int dst_size,
                         int* out_size_read);

    using OpenFilePtr = decltype(&CpkHandler::OpenFile);
    using CloseFilePtr = decltype(&CpkHandler::CloseFile);
    using ReadPtr = decltype(&CpkHandler::Read);

    static void Hook();
};
STATIC_ASSERT(sizeof(CpkHandler) == 0x50);


#endif
