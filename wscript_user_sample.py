import os
compilers = ['gcc', 'clang', 'clang-msvc', 'clang-msvc64']
configs = ['debug', 'rel-test', 'rel']

import itertools
variants = list(map(lambda x: '%s-%s' % x, itertools.product(compilers, configs)))

def my_configure(cfg):
    bdir = cfg.path.abspath()

    gcc = 'gcc'
    gxx = 'g++'
    clang_bin = os.path.expanduser('~/llvm/prefix/bin/')
    clang_flags = [
        '-stdlib=libc++', '-ferror-limit=5', '-ftemplate-backtrace-limit=0',
        '-march=native'
    ]
    clang_linkflags = [
        '-stdlib=libc++', '-fuse-ld=lld', '-march=native'
    ]

    vcdir = '/mnt/msvc/vc12'
    # i386 and x86_64
    clang_win32_tgt = [ '-target', 'i386-pc-windows-msvc18' ]
    clang_win64_tgt = [ '-target', 'x86_64-pc-windows-msvc18' ]
    clang_win_cxxflags = [
        '-march=x86-64',
        '-fms-compatibility-version=18',
        '-Xclang', '-internal-isystem', '-Xclang', vcdir+'/include',
        '-Xclang', '-internal-isystem', '-Xclang', vcdir+'/win_sdk/include/um',
        '-Xclang', '-internal-isystem', '-Xclang', vcdir+'/win_sdk/include/shared',
        '-DDOCTEST_CONFIG_COLORS_ANSI',
        '-ferror-limit=5',
    ]
    clang_win32_linkflags = clang_win32_tgt + [
        '-fuse-ld=lld',
        '-L%s/lib' % vcdir,
        '-L%s/win_sdk/lib/winv6.3/um/x86' % vcdir,
    ]
    clang_win64_linkflags = clang_win64_tgt + [
        '-fuse-ld=lld',
        '-L%s/lib/amd64' % vcdir,
        '-L%s/win_sdk/lib/winv6.3/um/x64' % vcdir,
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
                cfg.environ.pop('HOST_CC', None)
                cfg.environ.pop('HOST_CXX', None)
                cfg.options.all_system = None
            elif comp == 'clang':
                cfg.env.AR = clang_bin+'llvm-ar'
                cfg.env.CC = clang_bin+'clang'
                cfg.env.CXX = clang_bin+'clang++'
                cfg.env.CXXFLAGS = clang_flags
                cfg.env.LINKFLAGS = clang_linkflags
                cfg.environ.pop('HOST_CC', None)
                cfg.environ.pop('HOST_CXX', None)
                cfg.options.all_system = 'bundle'
            else: # clang-msvc*
                cfg.env.AR = clang_bin+'llvm-ar'
                cfg.env.CC = clang_bin+'clang'
                cfg.env.CXX = clang_bin+'clang++'
                cfg.environ['HOST_CC'] = 'gcc'
                cfg.environ['HOST_CXX'] = 'g++'
                cfg.options.all_system = 'bundle'

                if comp == 'clang-msvc':
                    cfg.environ['WINRC'] = 'i686-w64-mingw32-windres'
                    cfg.env.CXXFLAGS = cfg.env.CFLAGS = \
                        clang_win32_tgt + clang_win_cxxflags
                    cfg.env.LINKFLAGS = clang_win32_linkflags
                elif comp == 'clang-msvc64':
                    cfg.environ['WINRC'] = 'x86_64-w64-mingw32-windres'
                    cfg.env.CXXFLAGS = cfg.env.CFLAGS = \
                        clang_win64_tgt + clang_win_cxxflags
                    cfg.env.LINKFLAGS = clang_win64_linkflags
                else:
                    error()

            cfg.options.optimize_ext = True
            if conf == 'debug':
                cfg.options.optimize = False
                cfg.options.release = False
                cfg.options.with_tests = True
            elif conf == 'rel-test':
                cfg.options.optimize = True
                cfg.options.release = True
                cfg.options.with_tests = True
            elif conf == 'rel':
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
