import os
compilers = ['gcc', 'clang', 'clang-msvc', 'clang-msvc64']
configs = ['debug', 'rel-test', 'rel']

import itertools
variants = list(map(lambda x: '%s-%s' % x, itertools.product(compilers, configs)))

def my_configure(cfg):
    bdir = cfg.path.abspath()

    gcc = 'gcc-7.3.0'
    gxx = 'g++-7.3.0'
    winrc = '%s/libshit/rc.sh' % bdir
    clang_bin = os.path.expanduser('~/llvm/prefix/bin/')
    clang_flags = [
        '-stdlib=libc++', '-ferror-limit=5', '-ftemplate-backtrace-limit=0',
        '-march=native'
    ]
    clang_linkflags = [
        '-stdlib=libc++', '-fuse-ld=gold', '-march=native'
    ]

    vcdir = '/mnt/msvc/vc12'
    clangcl_cxxflags = [
        '-Xclang', '-target-cpu', '-Xclang', 'x86-64',
        '-fms-compatibility-version=18',
        '-imsvc', vcdir+'/include',
        '-imsvc', vcdir+'/win_sdk/include/um',
        '-imsvc', vcdir+'/win_sdk/include/shared',
        '-DDOCTEST_CONFIG_COLORS_ANSI',
        '-ferror-limit=5',
    ]
    clangcl_lib32 = [
        '/libpath:%s/lib' % vcdir,
        '/libpath:%s/win_sdk/lib/winv6.3/um/x86' % vcdir,
    ]
    clangcl_lib64 = [
        '/libpath:%s/lib/amd64' % vcdir,
        '/libpath:%s/win_sdk/lib/winv6.3/um/x64' % vcdir,
    ]

    cfg.options.host_lua = 'luajit'
    cfg.options.lua_dll = True

    for comp in compilers:
        for conf in configs:
            var = '%s-%s' % (comp, conf)
            if var not in variants: continue

            cfg.setenv(var)
            if comp == 'gcc':
                cfg.env.AR = 'gcc-ar'
                cfg.env.CC = gcc
                cfg.env.CXX = gxx
                cfg.env.CXXFLAGS = ['-march=native']
                cfg.env.LINKFLAGS = ['-march=native']
                cfg.options.clang_hack = False
                cfg.environ.pop('HOST_CC', None)
                cfg.environ.pop('HOST_CXX', None)
                cfg.options.all_system = None
            elif comp == 'clang':
                cfg.env.AR = clang_bin+'llvm-ar'
                cfg.env.CC = clang_bin+'clang'
                cfg.env.CXX = clang_bin+'clang++'
                cfg.env.CXXFLAGS = clang_flags
                cfg.env.LINKFLAGS = clang_linkflags
                cfg.options.clang_hack = False
                cfg.environ.pop('HOST_CC', None)
                cfg.environ.pop('HOST_CXX', None)
                cfg.options.all_system = 'bundle'
            else: # msvc
                cfg.env.AR = clang_bin+'llvm-lib'
                cfg.env.CC = clang_bin+'clang-cl'
                cfg.env.CXX = clang_bin+'clang-cl'
                cfg.options.clang_hack = True
                cfg.environ['HOST_CC'] = 'gcc'
                cfg.environ['HOST_CXX'] = 'g++'
                cfg.options.all_system = 'bundle'

                cfg.env.WINRC = winrc
                cfg.env.LINK_CXX = clang_bin+'lld-link'
                if comp == 'clang-msvc':
                    cfg.env.WINRCFLAGS = '-m32'
                    cfg.env.CXXFLAGS = cfg.env.CFLAGS = ['-m32'] + clangcl_cxxflags
                    cfg.env.LINKFLAGS = clangcl_lib32
                elif comp == 'clang-msvc64':
                    cfg.env.WINRCFLAGS = '-m64'
                    cfg.env.CXXFLAGS = cfg.env.CFLAGS = clangcl_cxxflags
                    cfg.env.LINKFLAGS = clangcl_lib64
                else:
                    error()

            cfg.options.optimize_ext = True
            if conf == 'debug':
                cfg.options.debug = True
                cfg.options.optimize = False
                cfg.options.release = False
                cfg.options.with_tests = True
            elif conf == 'rel-test':
                cfg.options.debug = False
                cfg.options.optimize = True
                cfg.options.release = True
                cfg.options.with_tests = True
            elif conf == 'rel':
                cfg.options.debug = False
                cfg.options.optimize = True
                cfg.options.release = True
                cfg.options.with_tests = False
            else:
                error()
            configure(cfg)

from waflib.Configure import ConfigurationContext
class my_configure_cls(ConfigurationContext):
    cmd = 'my_configure'
    fun = 'my_configure'

from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext

def init(ctx):
    for x in variants:
        for y in (BuildContext, CleanContext, InstallContext, UninstallContext):
            name = y.__name__.replace('Context','').lower()
            class tmp(y):
                cmd = '%s-%s' % (name, x)
                variant = x


from waflib import Utils, Build
class buildall_ctx(Build.BuildContext):
    cmd = fun = 'buildall'
    def compile(self):
        pass

def buildall(ctx):
    _build_many(ctx, variants)

def _build_many(ctx, variants):
    from waflib import Options, Task
    sem = Utils.threading.Semaphore(Options.options.jobs)
    def with_sem(f):
        def f2(self):
            sem.acquire()
            f(self)
            sem.release()
        return f2
    Task.TaskBase.process = with_sem(Task.TaskBase.process)

    threads = []
    for var in variants:
        cls = type(Build.BuildContext)(var, (Build.BuildContext,), {'cmd': var, 'variant': var})
        bld = cls(top_dir=ctx.top_dir, out_dir=ctx.out_dir)
        bld.targets = ctx.targets
        t = Utils.threading.Thread()
        t.run = bld.execute
        threads.append(t)

    for t in threads: t.start()
    for t in threads: t.join()
