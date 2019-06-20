#include "version.hpp"

#include <libshit/options.hpp>

#include <taihen.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>

#include <fstream>
#include <vector>
#include <memory>

#define LIBSHIT_LOG_NAME "tai"
#include <libshit/logger_helper.hpp>

namespace
{
  struct FsBinderHandle
  {
    FsBinderHandle* parent;
    FsBinderHandle* left;
    FsBinderHandle* right;
    void* unk0;
    uint32_t unk1;
    uint32_t index;
    uint32_t usage;
    // ...
  };

  struct FileCpkData
  {
    void* fhan;
    const char* cpk_name;
    uint64_t offs;
    uint32_t compr_size, uncompr_size;
    uint32_t binder_index;
  };

  struct FileInfo
  {
    FileCpkData* dat;
    char* name;
    int id;
    int id_ignored;
    void* fhan;
    const char* cpk_name;
    uint64_t cpk_offs;
    uint32_t idx1, idx2;
    uint32_t chain_length;
    // ...
  };

  template <typename F> class TaiHook;
  template <typename Res, typename... Args>
  class TaiHook<Res(Args...)>
  {
  public:
    Res operator()(Args... args)
    {
      return (n->next ? n->next->func : n->old)(std::forward<Args>(args)...);
    };

    operator uintptr_t*() { return reinterpret_cast<uintptr_t*>(&n); }

  private:
    using FuncPtr = Res(*)(Args...);
    struct Node
    {
      Node* next;
      FuncPtr func, old;
    };
    Node* n;
  };
}

using FsBinderHandleCreateFun = int(*)(FsBinderHandle**);
static FsBinderHandleCreateFun fs_binder_handle_create =
  reinterpret_cast<FsBinderHandleCreateFun>(0x811ea9a3);

static void* dir_fhan = reinterpret_cast<void*>(0x8143ada0);

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
    INF << "Creating FsBinder handle: " << fs_binder_handle_create(&my_han)
        << std::endl;
    fs_binder_handle_create(&my_han);
    if (!my_han) return ret;
    my_han->usage = 3;
  }

  char buf[256];
  snprintf(buf, 256, "ux0:neptools%s", fi->name);
  SceIoStat stat;
  if (sceIoGetstat(buf, &stat) < 0) return ret;
  if (!SCE_S_ISREG(stat.st_mode)) return ret;

  // file exists, hook it
  DBG(1) << "Hooking file: " << buf << std::endl;

  fi->dat->fhan = dir_fhan;
  fi->dat->cpk_name = "ux0:neptools";
  fi->dat->offs = 0;
  fi->dat->compr_size = stat.st_size;
  fi->dat->uncompr_size = stat.st_size;
  fi->dat->binder_index = my_han->index;

  fi->cpk_name = "ux0:neptools";
  fi->cpk_offs = 0;
  fi->fhan = dir_fhan;
  fi->idx1 = fi->idx2 = my_han->index;
  if (han_out) *han_out = my_han;

  return ret;
}

extern "C" int _start() __attribute__ ((weak, alias ("module_start")));
extern "C" int module_start(SceSize, const void*)
{
  std::vector<std::string> argv;

  {
    std::ifstream is{"ux0:neptools/command_line.txt"};
    while (is.good())
    {
      std::string s;
      std::getline(is, s);
      if (!s.empty()) argv.push_back(std::move(s));
    }
  }

  int argc = argv.size();

  std::unique_ptr<const char*[]> cargv(new const char*[argc+1]);
  for (int i = 0; i < argc; ++i) cargv[i] = argv[i].c_str();
  cargv[argc] = nullptr;

  auto& pars = Libshit::OptionParser::GetGlobal();
  pars.SetVersion("NepTools vita plugin v" NEPTOOLS_VERSION);
  pars.SetUsage("[--options]");
  pars.FailOnNonArg();

  try { pars.Run(argc, cargv.get()); }
  catch (const Libshit::Exit& e)
  { return e.success ? SCE_KERNEL_START_SUCCESS : SCE_KERNEL_START_FAILED; }


  auto main_mod = reinterpret_cast<const char*>(TAI_MAIN_MODULE);

  tai_module_info_t tai_info;
  tai_info.size = sizeof(tai_info);
  auto ret = taiGetModuleInfo(main_mod, &tai_info);
  if (ret < 0) return SCE_KERNEL_START_FAILED;

  ret = taiHookFunctionOffset(get_file_info, tai_info.modid, 0, 0x1eb8f8, 1,
                              reinterpret_cast<void*>(&HookGetFileInfo));
  if (ret < 0)
  {
    ERR << "taiHookFunction failed: " << ret << std::endl;
    return SCE_KERNEL_START_FAILED;
  }

  return SCE_KERNEL_START_SUCCESS;
}
