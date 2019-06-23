#include "version.hpp"
#include "source.hpp"
#include "open.hpp"
#include "txt_serializable.hpp"

#include <libshit/options.hpp>
#include <libshit/memory_utils.hpp>

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <memory>
#include <psp2/appmgr.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <taihen.h>
#include <unistd.h>
#include <vector>

#define LIBSHIT_LOG_NAME "tai"
#include <libshit/logger_helper.hpp>

using namespace Neptools; // todo

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

  struct FileHandle
  {
    SceUID fd;
    // ...
  };

  struct FileCpkData
  {
    FileHandle* fhan;
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
    FileHandle* fhan;
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

using UncompressFun = uint64_t (*)(char*, char*, uint64_t, char*, uint64_t);
static auto uncompress = reinterpret_cast<UncompressFun>(0x812480b1);

static std::string data_path = "ux0:neptools";
static std::string cache_path = "ux0:neptools/cache";

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


extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

static void uncaught()
{
  printf("Terminate handler\n");
  auto curr = std::current_exception();
  try { std::rethrow_exception(curr); }
  catch (const std::exception& e) { printf("Caught std: %s\n", e.what()); }
  catch (...) { printf("Caught ...\n"); }

  fflush(stdout);
  fflush(stderr);
  abort();
}

extern "C" int _start() __attribute__ ((weak, alias ("module_start")));
extern "C" int module_start(SceSize, const void*)
{
  freopen("ux0:neptools/log_out.txt", "w", stdout);
  freopen("ux0:neptools/log_err.txt", "w", stderr);

  std::set_terminate(uncaught);

  pthread_init();
  for (auto ptr = __preinit_array_start; ptr != __preinit_array_end; ++ptr)
    (*ptr)();
  for (auto ptr = __init_array_start; ptr != __init_array_end; ++ptr)
    (*ptr)();

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

  int argc = argv.size() + 1;

  std::unique_ptr<const char*[]> cargv(new const char*[argc+2]);
  cargv[0] = "";
  for (int i = 1; i < argc; ++i) cargv[i] = argv[i-1].c_str();
  cargv[argc+1] = nullptr;

  auto& pars = Libshit::OptionParser::GetGlobal();
  pars.SetVersion("NepTools vita plugin v" NEPTOOLS_VERSION);
  pars.SetUsage("[--options]");
  pars.FailOnNonArg();

  try { pars.Run(argc, cargv.get()); }
  catch (const Libshit::Exit& e) { _exit(!e.success); }

  //INF << "Erasing cache" << std::endl;
  //boost::filesystem::remove_all("ux0:neptools/cache");

  auto main_mod = reinterpret_cast<const char*>(TAI_MAIN_MODULE);

  tai_module_info_t tai_info;
  tai_info.size = sizeof(tai_info);
  auto ret = taiGetModuleInfo(main_mod, &tai_info);
  if (ret < 0)
  {
    ERR << "taiGetModuleInfo failed: " << ret << std::endl;
    return SCE_KERNEL_START_FAILED;
  }

  SceKernelModuleInfo kern_info;
  kern_info.size = sizeof(kern_info);
  ret = sceKernelGetModuleInfo(tai_info.modid, &kern_info);
  if (ret < 0)
  {
    ERR << "sceKernelGetModuleInfo failed: " << ret << std::endl;
    return SCE_KERNEL_START_FAILED;
  }

  DBG(1) << "Base: " << kern_info.segments[0].vaddr << ", size: "
         << kern_info.segments[0].size << std::endl;

  char app_id[10];
  ret = sceAppMgrAppParamGetString(sceKernelGetProcessId(), 12, app_id, 10);
  if (ret < 0)
  {
    ERR << "sceAppMgrAppParamGetString failed" << std::endl;
    return SCE_KERNEL_START_FAILED;
  }
  INF << "AppID: " << app_id << std::endl;

  ret = taiHookFunctionOffset(get_file_info, tai_info.modid, 0, 0x1eb8f8, 1,
                              reinterpret_cast<void*>(&HookGetFileInfo));
  if (ret < 0)
  {
    ERR << "taiHookFunction failed: " << ret << std::endl;
    return SCE_KERNEL_START_FAILED;
  }

  return SCE_KERNEL_START_SUCCESS;
}
