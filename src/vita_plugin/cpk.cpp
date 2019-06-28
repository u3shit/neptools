#include "vita_plugin/cpk.hpp"

#include "open.hpp"
#include "source.hpp"
#include "txt_serializable.hpp"
#include "vita_plugin/taihen_cpp.hpp"

#include <libshit/memory_utils.hpp>

#include <cstdint>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <psp2/kernel/modulemgr.h>
#include <stdexcept>

#define LIBSHIT_LOG_NAME "cpk"
#include <libshit/logger_helper.hpp>

namespace Neptools::VitaPlugin
{

  namespace
  {
    struct FsBinderHandle
    {
      FsBinderHandle* parent;
      FsBinderHandle* left;
      FsBinderHandle* right;
      void* unk0;
      std::uint32_t unk1;
      std::uint32_t index;
      std::uint32_t usage;
      // ...
    };

    struct FileHandle
    {
      std::int32_t fd;
      // ...
    };

    struct FileCpkData
    {
      FileHandle* fhan;
      const char* cpk_name;
      std::uint64_t offs;
      std::uint32_t compr_size, uncompr_size;
      std::uint32_t binder_index;
    };

    struct FileInfo
    {
      FileCpkData* dat;
      char* name;
      std::uint32_t id;
      std::uint32_t id_ignored;
      FileHandle* fhan;
      const char* cpk_name;
      std::uint64_t cpk_offs;
      std::uint32_t idx1, idx2;
      std::uint32_t chain_length;
      // ...
    };
  }

  static std::string data_path;
  static std::string cache_path;

  using UncompressFun = uint64_t (*)(char*, char*, uint64_t, char*, uint64_t);
  static auto uncompress = reinterpret_cast<UncompressFun>(0x812480b1);

  static Source GetSource(const FileCpkData& dat)
  {
    auto src = Source::FromFd(dat.cpk_name, dat.fhan->fd, false);
    src.Slice(dat.offs, dat.compr_size);
    if (dat.compr_size == dat.uncompr_size) return src;

    auto buf = Libshit::MakeUnique<char[]>(dat.uncompr_size, Libshit::uninitialized);
    src.Pread(0, buf.get(), dat.compr_size);

    char tmp[0x104];
    auto res = uncompress(tmp, buf.get(), dat.compr_size, buf.get(),
                          dat.uncompr_size);
    if (res != dat.uncompr_size)
      LIBSHIT_THROW(std::runtime_error, "Data decompression failed");

    return Source::FromMemory("", Libshit::Move(buf), dat.uncompr_size);
  }

  static std::pair<const char*, size_t> DoTxt(
    const char* fname, const FileCpkData& dat)
  {
    auto cpth = cache_path + fname;
    if (boost::filesystem::exists(cpth))
    {
      DBG(3) << "Cached exists: " << cpth << std::endl;
      return {cache_path.c_str(), boost::filesystem::file_size(cpth)};
    }

    auto pth = data_path + fname + ".txt";
    if (!boost::filesystem::exists(pth)) return {nullptr, 0};

    auto dir = boost::filesystem::path(cpth).remove_filename();
    DBG(4) << "Mkdir " << dir << std::endl;
    boost::filesystem::create_directories(dir);

    DBG(3) << "Txt exists: " << pth << std::endl;
    auto dump = OpenFactory::Open(GetSource(dat));
    auto txt = dump->GetDefaultTxtSerializable(dump);
    txt->ReadTxt(OpenIn(pth));
    dump->Fixup();
    dump->Dump(Libshit::Move(cpth));

    return {cache_path.c_str(), dump->GetSize()};
  }

  static std::pair<const char*, size_t> DoBin(const char* fname)
  {
    auto pth = data_path + fname;
    if (!boost::filesystem::exists(pth)) return {nullptr, 0};
    DBG(3) << "Simple exists " << pth << std::endl;
    return {data_path.c_str(), boost::filesystem::file_size(pth)};
  }

  using FsBinderHandleCreateFun = int(*)(FsBinderHandle**);
  static FsBinderHandleCreateFun fs_binder_handle_create =
    reinterpret_cast<FsBinderHandleCreateFun>(0x811ea9a3);

  static FileHandle* dir_fhan = reinterpret_cast<FileHandle*>(0x8143ada0);

  static TaiHook<int(FsBinderHandle*, FileInfo*, FsBinderHandle**, int*)>
  get_file_info;
  static FsBinderHandle* my_han;

  static int HookGetFileInfo(
    FsBinderHandle* han, FileInfo* fi, FsBinderHandle** han_out, int* succ)
  {
    auto ret = get_file_info(han, fi, han_out, succ);

    if (ret != 0 || *succ != 1 || !fi->dat) return ret;
    if (!my_han)
    {
      DBG(0) << "Creating FsBinder handle: " << fs_binder_handle_create(&my_han)
          << std::endl;
      fs_binder_handle_create(&my_han);
      if (!my_han) return ret;
      my_han->usage = 3;
    }

    DBG(2) << "Checking file: " << fi->name << std::endl;
    auto r = DoBin(fi->name);
    if (!r.first) r = DoTxt(fi->name, *fi->dat);
    if (!r.first) return ret;

    DBG(1) << "Hooked file: " << fi->name << std::endl;

    fi->dat->fhan = dir_fhan;
    fi->dat->cpk_name = r.first;
    fi->dat->offs = 0;
    fi->dat->compr_size = r.second;
    fi->dat->uncompr_size = r.second;
    fi->dat->binder_index = my_han->index;

    fi->cpk_name = r.first;
    fi->cpk_offs = 0;
    fi->fhan = dir_fhan;
    fi->idx1 = fi->idx2 = my_han->index;
    if (han_out) *han_out = my_han;

    return ret;
  }

  void Init(std::string data_path_in, std::string cache_path_in)
  {
    data_path = Libshit::Move(data_path_in);
    cache_path = Libshit::Move(cache_path_in);

    INF << "Neptools Vita Initializing...\nData path: " << data_path
        << "\nCache path: " << cache_path << std::endl;

    INF << "Erasing cache..." << std::endl;
    boost::filesystem::remove_all(cache_path);

    tai_module_info_t tai_info;
    tai_info.size = sizeof(tai_info);
    auto ret = taiGetModuleInfo(TAI_MAIN_MOD_STR, &tai_info);
    if (ret < 0)
      LIBSHIT_THROW(TaiError, "taiGetModuleInfo failed", "Error code", ret);

    SceKernelModuleInfo kern_info;
    kern_info.size = sizeof(kern_info);
    ret = sceKernelGetModuleInfo(tai_info.modid, &kern_info);
    if (ret < 0)
      LIBSHIT_THROW(TaiError, "sceKernelGetModuleInfo failed", "Error code", ret);

    DBG(1) << "Base: " << kern_info.segments[0].vaddr << ", size: "
           << kern_info.segments[0].size << std::endl;

    ret = taiHookFunctionOffset(get_file_info, tai_info.modid, 0, 0x1eb8f8, 1,
                                reinterpret_cast<void*>(&HookGetFileInfo));
    if (ret < 0)
      LIBSHIT_THROW(TaiError, "Hook GetFileInfo failed", "Error code", ret);
  }

}
