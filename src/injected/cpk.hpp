#ifndef INJECTED_CPK_HPP
#define INJECTED_CPK_HPP
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

using OperatorNewPtr = void* (__cdecl *)(size_t size);
extern OperatorNewPtr operator_new;
using OperatorDeletePtr = void (__cdecl *)(void* ptr);
extern OperatorDeletePtr operator_delete;

template <class T>
struct HostAllocator
{
    using value_type = T;

    T* allocate(size_t n) { return static_cast<T*>(operator_new(n)); }
    void deallocate(T* p, size_t) { operator_delete(p); }
};

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
static_assert(sizeof(PakEntry) == 0x120, "");

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
static_assert(sizeof(CpkHandlerFileInfo) == 0x140, "");

struct CpkHandler
{
    unsigned vect0_begin, vect0_end, vect0_capacity;
#if _MSC_VER != 1900
#error Check MSC ver
#endif
    using FileInfo = CpkHandlerFileInfo;
    std::vector<FileInfo*, HostAllocator<FileInfo*>> entry_vect;
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
static_assert(sizeof(CpkHandler) == 0x50, "");


#endif
