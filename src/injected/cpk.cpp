#include "cpk.hpp"
#include "hook.hpp"
#include "../fs.hpp"
#include "../pattern_parse.hpp"

#include "../format/cl3.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"

#include <iostream>

namespace
{

auto OPEN_FILE  = "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 81 ec 3c "
                  "06 00 00"_pattern;
auto FILE_CLOSE = "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 51 53 56 "
                  "57 a1 ?? ?? ?? ?? 33 c5 50 8d 45 f4 64 a3 00 00 00 00 8b f9 "
                  "8d 77 38 56 89 75 f0 ff 15 ?? ?? ?? ?? c7 45 fc 00 00 00 00 "
                  "8b 47 10"_pattern;
auto FILE_READ  = "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 51 53 56 "
                  "57 a1 ?? ?? ?? ?? 33 c5 50 8d 45 f4 64 a3 00 00 00 00 8b d9 "
                  "8d 73 38 56 89 75 f0 ff 15 ?? ?? ?? ?? c7 45 fc 00 00 00 00 "
                  "8b 55 08"_pattern;

constexpr const FileMemSize CPK_CHUNK = 128*1024*1024; // default size used by if

struct CpkSource : public Source::Provider
{
    CpkSource(fs::path fname, CpkHandler* cpk, size_t index);
    ~CpkSource();
    void Pread(FilePosition offs, Byte* buf, FileMemSize len) override;

    CpkHandler* cpk;
    size_t index;
};

}

CpkSource::CpkSource(fs::path fname, CpkHandler* cpk, size_t index)
    : Source::Provider{
          std::move(fname), cpk->entry_vect[index]->entry.uncompressed_size},
      cpk{cpk}, index{index}
{
}

CpkSource::~CpkSource()
{
    for (auto& e : lru)
        delete[] e.ptr;
    cpk->OrigCloseFile(index);
}

void CpkSource::Pread(FilePosition offs, Byte* buf, FileMemSize len)
{
    if (len > CPK_CHUNK)
    {
        size_t read;
        cpk->entry_vect[index]->read_pos = offs;
        if (!cpk->OrigRead(index, reinterpret_cast<char*>(buf), len, &read))
            THROW(CpkError{"Cpk::OrigRead failed"} <<
                  CpkErrorCode{cpk->last_error});
        BOOST_ASSERT(read == len);
        return;
    }

    do
    {
        auto coffs = offs / CPK_CHUNK * CPK_CHUNK;
        auto csize = std::min(CPK_CHUNK, size-coffs);
        std::unique_ptr<char[]> cbuf{new char[csize]};

        size_t read;
        cpk->entry_vect[index]->read_pos = coffs;
        if (!cpk->OrigRead(index, cbuf.get(), csize, &read))
            THROW(CpkError{"Cpk::OrigRead failed"} <<
                  CpkErrorCode{cpk->last_error});
        BOOST_ASSERT(read == csize);

        auto to_offs = offs % CPK_CHUNK;
        auto to_copy = std::min(len, csize - to_offs);
        memcpy(buf, cbuf.get() + to_offs, to_copy);
        delete[] lru[lru.size()-1].ptr;
        LruPush(reinterpret_cast<Byte*>(cbuf.release()), offs, csize);

        buf += to_copy;
        offs += to_copy;
        len -= to_copy;
    }
    while (len);
}

CpkHandler::OpenFilePtr CpkHandler::orig_open_file;
CpkHandler::CloseFilePtr CpkHandler::orig_close_file;
CpkHandler::ReadPtr CpkHandler::orig_read;

char CpkHandler::OpenFile(const char* fname, size_t* out)
{
#ifndef NDEBUG
    std::cerr << "OpenFile " << basename << fname;
#endif
    fs::path pth{"kitfolder"};
    pth /= basename;
    pth /= fname;

    if (OpenTxtFile(fname, pth, out) || OpenFsFile(fname, pth, out))
    {
#ifndef NDEBUG
        std::cerr << " -- hooked it" << std::endl;
#endif
        return 1;
    }
    else
    {
        auto ret = (this->*orig_open_file)(fname, out);
#ifndef NDEBUG
        std::cerr << " -> " << int(ret) << ", " << *out << std::endl;
#endif
        return ret;
    }
}

bool CpkHandler::OpenFsFile(const char* fname, const fs::path& pth, size_t* out)
{
    auto h = CreateFileW(pth.c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE)
    {
#ifndef NDEBUG
        std::cerr << " -- can't fs hook: CreateFileW failed: " << GetLastError();
#endif
        return false;
    }
    size_t size = GetFileSize(h, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
#ifndef NDEBUG
        std::cerr << " -- can't fs hook: GetFileSize failed: " << GetLastError();
#endif
        CloseHandle(h);
        return false;
    }

    // success
    auto& info = GetEntryVect(out);
    info.handle = h;
    strncpy(info.entry.file_name, fname, 260);
    info.entry.compressed_size = size;
    info.entry.uncompressed_size = size;

    return true;
}

bool CpkHandler::OpenTxtFile(const char* fname, const fs::path& pth, size_t* out)
{
    FileMemSize size;
    std::unique_ptr<Byte[]> buf;

    try
    {
        fs::path pthtxt = pth;
        if (!fs::exists(pthtxt += ".txt")) return false;

        Source src{fs::exists(pth) ? Source::FromFile(pth) : GetSource(fname)};
        if (src.GetSize() < 4)
            THROW(DecodeError{"Input file too short"});

        char hdr_buf[4];
        src.Pread(0, hdr_buf, 4);

        std::unique_ptr<Dumpable> dmp;
        Gbnl* gbnl;
        if (memcmp(hdr_buf, "CL3L", 4) == 0)
        {
            dmp = std::make_unique<Cl3>(src);
            gbnl = &static_cast<Cl3*>(dmp.get())->GetStcm().FindGbnl();
        }
        else
        {
            dmp = std::make_unique<Gbnl>(src);
            gbnl = static_cast<Gbnl*>(dmp.get());
        }

        gbnl->ReadTxt(OpenIn(pthtxt));
        dmp->Fixup();
        size = dmp->GetSize();
        buf.reset(new Byte[size]);
        dmp->Dump(MemorySink{buf.get(), size});
    }
    catch (const std::exception& e)
    {
#ifndef NDEBUG
        std::cerr << " -- txt hook failed: ";
        PrintException(std::cerr);
#endif
        return false;
    }

    //dbg
    /*
    std::string pths = pth.string();
    boost::replace_all(pths, "/", "_");
    boost::replace_all(pths, "\\", "_");
    std::ofstream os{pths, std::ios_base::binary};
    os.write(reinterpret_cast<char*>(buf.get()), size);
    */
    //end dbg

    auto& info = GetEntryVect(out);
    strncpy(info.entry.file_name, fname, 260);
    info.handle = INVALID_HANDLE_VALUE;
    info.entry.compressed_size = size;
    info.entry.uncompressed_size = size;
    info.block = buf.release();
    return true;
}

CpkHandlerFileInfo& CpkHandler::GetEntryVect(size_t* out)
{
    auto it = std::find_if(entry_vect.begin(), entry_vect.end(),
                           [](auto& x) { return x->is_valid == 0; });
    if (it == entry_vect.end())
    {
        entry_vect.reserve(entry_vect.size() + 1);
        auto ne = new CpkHandlerFileInfo;
        ne->huffmann_hdr = nullptr;
        ne->block = nullptr;
        entry_vect.push_back(ne);
        it = entry_vect.end() - 1;
    }

    (*it)->index = 0;
    (*it)->is_valid = true;
    (*it)->entry.field_000 = 0;
    (*it)->entry.file_index = 0;
    (*it)->entry.field_10c = 0;
    (*it)->entry.is_compressed = 0;
    (*it)->entry.field_11c = 0;
    (*it)->data_start = 0;
    (*it)->read_pos = 0;
    (*it)->decoded_block_index = -1;

    if (out) *out = it - entry_vect.begin();
    return **it;
}

char CpkHandler::CloseFile(unsigned index)
{
#ifndef NDEBUG
    //std::cerr << "CloseFile " << index << std::endl;
#endif
    return OrigCloseFile(index);
}

char CpkHandler::Read(unsigned index, char* dst, size_t dst_size,
                      size_t* out_size_read)
{
#ifndef NDEBUG
    //std::cerr << "Read " << index << " " << dst_size << std::endl;
#endif
    auto& en = *entry_vect[index];
    if (en.handle == INVALID_HANDLE_VALUE)
    {
        // cpk from mem
#ifndef NDEBUG
        std::cerr << "cpk read " << en.read_pos << " " << dst_size << std::endl;
#endif
        dst_size = std::min(dst_size, en.entry.uncompressed_size - en.read_pos);
        memcpy(dst, static_cast<char*>(en.block) + en.read_pos, dst_size);
        en.read_pos += dst_size;
        if (out_size_read) *out_size_read = dst_size;
        return 1;
    }
    return OrigRead(index, dst, dst_size, out_size_read);
}

Source CpkHandler::GetSource(const char* fname)
{
    size_t index;
    if (!OrigOpenFile(fname, &index))
            THROW(CpkError{"Cpk::OrigOpenFile failed"} <<
                  CpkErrorCode{last_error});
    try
    {
        return Source(std::make_shared<CpkSource>(fname, this, index),
                      entry_vect[index]->entry.uncompressed_size);
    }
    catch (...)
    {
        OrigCloseFile(index);
        throw;
    }
}

void CpkHandler::Hook()
{
    orig_open_file = ::Hook(FindImage(OPEN_FILE), &CpkHandler::OpenFile, 5);
    orig_close_file = ::Hook(FindImage(FILE_CLOSE), &CpkHandler::CloseFile, 5);
    orig_read = ::Hook(FindImage(FILE_READ), &CpkHandler::Read, 5);
}
