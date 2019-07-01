#include "version.hpp"
#include "vita_plugin/cpk.hpp"

#include <libshit/except.hpp>
#include <libshit/memory_utils.hpp>
#include <libshit/options.hpp>
#include <libshit/vita_fixup.h>

#include <fstream>
#include <memory>
#include <psp2/appmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>
#include <unistd.h>
#include <vector>

#define LIBSHIT_LOG_NAME "tai"
#include <libshit/logger_helper.hpp>

namespace Neptools::VitaPlugin
{

  static void Uncaught()
  {
    printf("Terminate handler\n");
    printf("Caught: %s\n", Libshit::ExceptionToString().c_str());

    abort();
  }

  static bool CheckPath(const char* buf, const char*& out)
  {
    SceIoStat stat;
    auto ret = sceIoGetstat(buf, &stat);
    if (ret == 0 && SCE_S_ISDIR(stat.st_mode))
    {
      out = buf;
      return true;
    }
    return false;
  }

  static void my_strlcpy(char* dst, const char* str, size_t n)
  {
    while (--n && (*dst++ = *str++));
    *dst = 0;
  }

  static int FindDir(char* out_base_fname, char* out_data_fname, char* out_cache_fname)
  {
    char log_buf[32];
    log_buf[0] = 0;

    char buf[] = "____:neptools/_________";
    constexpr size_t off = sizeof("____:neptools/")-1;
    int ret = sceAppMgrAppParamGetString(sceKernelGetProcessId(), 12, buf+off, 10);
    if (ret < 0) return ret; // can't log anything here...

    // same order as repatch
    const char* base_path = nullptr;
    bool cache_td = false;
    for (const auto& x : {"ux0", "uma0", "imc0", "grw0", "xmc0"})
    {
      size_t len = strlen(x);
      memcpy(buf + 4 - len, x, len);
      if (CheckPath(buf + 4 - len, base_path))
      {
        my_strlcpy(log_buf, base_path, buf+off+1-base_path);
        break;
      }
    }
    if (!base_path && CheckPath("app0:neptools", base_path)) cache_td = true;
    if (!base_path) return -1;
    my_strlcpy(out_data_fname, base_path, 32);

    // check for ioplus bug
    if (!cache_td)
    {
      ret = sceIoDopen(base_path);
      if (ret >= 0) sceIoDclose(ret);
      else cache_td = true;
    }

    char td_buf[16];
    if (cache_td)
    {
      ret = sceAppMgrWorkDirMount(0xc9, td_buf);
      if (ret < 0) return ret;
    }
    if (!log_buf[0]) strcpy(log_buf, td_buf);

    if (cache_td)
    {
      strcpy(out_cache_fname, td_buf);
      strcat(out_cache_fname, "neptools_cache");
    }
    else
    {
      my_strlcpy(out_cache_fname, base_path, strlen(base_path) - 9 + 1);
      strcat(out_cache_fname, "cache");
    }

    strcpy(out_base_fname, log_buf);
    auto pos = log_buf + strlen(log_buf);
    strcpy(pos, "log_out.txt");
    freopen(log_buf, "w", stdout);
    strcpy(pos, "log_err.txt");
    freopen(log_buf, "w", stderr);

    return 0;
  }

  extern "C" int _start() __attribute__ ((weak, alias ("module_start")));
  extern "C" int module_start(SceSize, const void*)
  {
    char base_fname[32], data_fname[32], cache_fname[32];
    if (FindDir(base_fname, data_fname, cache_fname) < 0)
      return SCE_KERNEL_START_FAILED;

    std::set_terminate(Uncaught);

    LibshitInitVita();

    std::vector<std::string> argv;
    {
      strcat(base_fname, "/command_line.txt");
      std::ifstream is{base_fname};
      while (is.good())
      {
        std::string s;
        std::getline(is, s);
        if (!s.empty()) argv.push_back(std::move(s));
      }
    }

    int argc = argv.size() + 1;

    auto cargv = Libshit::MakeUnique<const char*[]>(
      argc + 2, Libshit::uninitialized);
    cargv[0] = "";
    for (int i = 1; i < argc; ++i) cargv[i] = argv[i-1].c_str();
    cargv[argc+1] = nullptr;

    auto& pars = Libshit::OptionParser::GetGlobal();
    pars.SetVersion("NepTools vita plugin v" NEPTOOLS_VERSION);
    pars.SetUsage("[--options]");
    pars.FailOnNonArg();

    try { pars.Run(argc, cargv.get()); }
    catch (const Libshit::Exit& e) { _exit(!e.success); }

    INF << pars.GetVersion() << " initializing..." << std::endl;
    Init(data_fname, cache_fname);

    return SCE_KERNEL_START_SUCCESS;
  }

}
