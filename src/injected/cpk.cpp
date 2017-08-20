#include "cpk.hpp"
#include "hook.hpp"
#include "../pattern_parse.hpp"

#include "../format/cl3.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"

#include <boost/filesystem/operations.hpp>
#include <iostream>

#define LIBSHIT_LOG_NAME "cpk"
#include <libshit/logger_helper.hpp>

namespace Neptools
{

  static auto OPEN_FILE  = NEPTOOLS_PATTERN(
    "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 81 ec 3c 06 00 00");

  static auto FILE_CLOSE = NEPTOOLS_PATTERN(
    "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 51 53 56 "
    "57 a1 ?? ?? ?? ?? 33 c5 50 8d 45 f4 64 a3 00 00 00 00 8b f9 "
    "8d 77 38 56 89 75 f0 ff 15 ?? ?? ?? ?? c7 45 fc 00 00 00 00 "
    "8b 47 10");
  static auto FILE_CLOSE_RB1PATCH = NEPTOOLS_PATTERN(
    "55 8b ec 53 56 57 8b f9 8d 77 38 56 ff 15 ?? ?? ?? ?? 8b 4f 0c");

  static auto FILE_READ = NEPTOOLS_PATTERN(
    "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 51 53 56 "
    "57 a1 ?? ?? ?? ?? 33 c5 50 8d 45 f4 64 a3 00 00 00 00 8b d9 "
    "8d 73 38 56 89 75 f0 ff 15 ?? ?? ?? ?? c7 45 fc 00 00 00 00 "
    "8b 55 08");
  static auto FILE_READ_RB1PATCH = NEPTOOLS_PATTERN(
    "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 51 53 56 57 a1 ?? ?? "
    "?? ?? 33 c5 50 8d 45 f4 64 a3 00 00 00 00 8b d9 8d 73 38 56 89 75 f0 ff "
    "15 ?? ?? ?? ?? 8b 55 08");

  // default size used by if
  constexpr const FileMemSize CPK_CHUNK = 128*1024*1024;

  namespace
  {
    struct CpkSource : public Source::Provider
    {
      CpkSource(boost::filesystem::path fname, CpkHandler* cpk, size_t index);
      ~CpkSource();
      void Pread(FilePosition offs, Byte* buf, FileMemSize len) override;

      CpkHandler* cpk;
      size_t index;
    };
  }

  CpkSource::CpkSource(
    boost::filesystem::path fname, CpkHandler* cpk, size_t index)
    : Source::Provider{
        std::move(fname), cpk->entry_vect[index]->entry.uncompressed_size},
      cpk{cpk}, index{index}
  {}

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
        LIBSHIT_THROW(CpkError{"Cpk::OrigRead failed"} <<
                      CpkErrorCode{cpk->last_error});
      LIBSHIT_ASSERT(read == len);
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
        LIBSHIT_THROW(CpkError{"Cpk::OrigRead failed"} <<
                      CpkErrorCode{cpk->last_error});
      LIBSHIT_ASSERT(read == csize);

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
    DBG(2) << "OpenFile " << basename << fname;
    boost::filesystem::path pth{"neptools"};
    pth /= basename;
    pth /= fname;

    if (OpenTxtFile(fname, pth, out) || OpenFsFile(fname, pth, out))
    {
      DBG(2) << " -- hooked it" << std::endl;
      return 1;
    }
    else
    {
      auto ret = (this->*orig_open_file)(fname, out);
      DBG(2) << " -> " << int(ret) << ", " << *out << std::endl;
      return ret;
    }
  }

  bool CpkHandler::OpenFsFile(
    const char* fname, const boost::filesystem::path& pth, size_t* out)
  {
    auto h = CreateFileW(pth.c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (h == INVALID_HANDLE_VALUE)
    {
      DBG(2) << " -- can't fs hook: CreateFileW failed: " << GetLastError();
      return false;
    }
    size_t size = GetFileSize(h, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
      DBG(2) << " -- can't fs hook: GetFileSize failed: " << GetLastError();
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

  bool CpkHandler::OpenTxtFile(
    const char* fname, const boost::filesystem::path& pth, size_t* out)
  {
    FileMemSize size;
    std::unique_ptr<Byte[]> buf;

    try
    {
      boost::filesystem::path pthtxt = pth;
      if (!boost::filesystem::exists(pthtxt += ".txt")) return false;

      Source src{boost::filesystem::exists(pth) ?
          Source::FromFile(pth) : GetSource(fname)};
      src.CheckSize(4);

      char hdr_buf[4];
      src.Pread(0, hdr_buf, 4);

      Libshit::SmartPtr<Dumpable> dmp;
      TxtSerializable* txt;
      if (memcmp(hdr_buf, "CL3L", 4) == 0)
      {
        dmp = Libshit::MakeSmart<Cl3>(src);
        txt = &static_cast<Cl3*>(dmp.get())->GetStcm();
      }
      else
      {
        dmp = Libshit::MakeSmart<Gbnl>(src);
        txt = static_cast<Gbnl*>(dmp.get());
      }

      txt->ReadTxt(OpenIn(pthtxt));
      dmp->Fixup();
      size = dmp->GetSize();
      buf.reset(new Byte[size]);
      dmp->Dump(MemorySink{buf.get(), size});
    }
    catch (const std::exception& e)
    {
      auto except = Libshit::ExceptionToString();
      DBG(2) << " -- txt hook failed: " << except << std::endl;

      std::stringstream ss;
      ss << "Failed to import " << pth << ", ignoring.\n\n" << except;
      MessageBoxA(nullptr, ss.str().c_str(), "Neptools", MB_OK | MB_ICONERROR);
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
    DBG(3) << "CloseFile " << index << std::endl;
    return OrigCloseFile(index);
  }

  char CpkHandler::Read(unsigned index, char* dst, size_t dst_size,
                        size_t* out_size_read)
  {
    DBG(3) << "Read " << index << " " << dst_size << std::endl;

    auto& en = *entry_vect[index];
    if (en.handle == INVALID_HANDLE_VALUE)
    {
      // cpk from mem
      DBG(3) << "cpk read " << en.read_pos << " " << dst_size << std::endl;

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
      LIBSHIT_THROW(CpkError{"Cpk::OrigOpenFile failed"} <<
                    CpkErrorCode{last_error});
    try
    {
      return Source(Libshit::MakeSmart<CpkSource>(fname, this, index),
                    entry_vect[index]->entry.uncompressed_size);
    }
    catch (...)
    {
      OrigCloseFile(index);
      throw;
    }
  }

  void CpkHandler::Init()
  {
    DBG(1) << "Finding OPEN_FILE" << std::endl;
    orig_open_file = Hook(FindImage(OPEN_FILE), &CpkHandler::OpenFile, 5);

    DBG(1) << "Finding FILE_CLOSE" << std::endl;
    auto offs = MaybeFindImage(FILE_CLOSE);
    if (!offs) offs = FindImage(FILE_CLOSE_RB1PATCH);
    orig_close_file = Hook(offs, &CpkHandler::CloseFile, 5);

    DBG(1) << "Finding FILE_READ" << std::endl;
    offs = MaybeFindImage(FILE_READ);
    if (!offs) offs = FindImage(FILE_READ_RB1PATCH);
    orig_read = Hook(offs, &CpkHandler::Read, 5);
  }

}
