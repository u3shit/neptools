import os
variants = ['gcc-debug', 'gcc-rel', 'clang-debug', 'clang-rel', 'clang-msvc',
            'clang-msvc-debug', 'clang-msvc64']

def my_configure(cfg):
    bdir = cfg.path.abspath()

    gcc = 'gcc-5.4.0'
    gxx = 'g++-5.4.0'
    winrc = '%s/rc.sh' % bdir
    clang_bin = os.path.expanduser('~/llvm/prefix/bin/')

    cfg.environ['HOST_CC'] = 'gcc'
    cfg.environ['HOST_CXX'] = 'g++'
    cfg.options.host_lua = 'luajit'

    if 'gcc-debug' in variants:
        cfg.setenv('gcc-debug')
        cfg.env.CC = gcc
        cfg.env.CXX = gxx
        cfg.env.CXXFLAGS = '-ggdb3'
        cfg.env.LINKFLAGS = ['-ggdb3']
        cfg.options.optimize = False
        cfg.options.optimize_ext = True
        cfg.options.release = False
        configure(cfg)

    if 'gcc-rel' in variants:
        cfg.setenv('gcc-rel')
        cfg.env.AR = 'gcc-ar'
        cfg.env.CC = gcc
        cfg.env.CXX = gxx
        cfg.env.CXXFLAGS = '-march=native'
        cfg.env.LINKFLAGS = ['-march=native']
        cfg.options.optimize = True
        cfg.options.release = True
        configure(cfg)

    if 'clang-debug' in variants:
        cfg.setenv('clang-debug')
        cfg.env.CC = clang_bin+'clang'
        cfg.env.CXX = clang_bin+'clang++'
        cfg.env.CXXFLAGS = ['-stdlib=libc++', '-ggdb3']
        cfg.env.LINKFLAGS = ['-stdlib=libc++', '-ggdb3']
        cfg.options.optimize = False
        cfg.options.optimize_ext = True
        cfg.options.release = False
        configure(cfg)

    if 'clang-rel' in variants:
        cfg.setenv('clang-rel')
        cfg.env.CC = clang_bin+'clang'
        cfg.env.CXX = clang_bin+'clang++'
        cfg.env.CXXFLAGS = ['-stdlib=libc++', '-march=native']
        cfg.env.LINKFLAGS = ['-stdlib=libc++', '-march=native', '-fuse-ld=gold']
        cfg.options.optimize = True
        cfg.options.release = True
        configure(cfg)

    clang_cxxflags = [
        '-Xclang', '-target-cpu', '-Xclang', 'x86-64',
        '-fms-compatibility-version=18',
        '-imsvc', '/mnt/msvc/vc12/include',
        '-imsvc', '/mnt/msvc/vc12/win_sdk/Include/um',
        '-imsvc', '/mnt/msvc/vc12/win_sdk/Include/shared',
    ]
    if 'clang-msvc' in variants:
        cfg.setenv('clang-msvc')
        cfg.env.AR = clang_bin+'llvm-lib'
        cfg.env.CC = clang_bin+'clang-cl'
        cfg.env.CXX = clang_bin+'clang-cl'
        cfg.env.WINRC = winrc
        cfg.env.WINRCFLAGS = '-m32'
        cfg.env.LINK_CXX = clang_bin+'lld-link'
        cfg.env.CXXFLAGS = cfg.env.CFLAGS = ['-m32'] + clang_cxxflags
        cfg.env.LINKFLAGS = [
            #'/opt:lldlto=0',
            '/libpath:/mnt/msvc/vc12/lib',
            '/libpath:/mnt/msvc/vc12/win_sdk/Lib/winv6.3/um/x86',
            #'/debug',
        ]
        cfg.options.clang_hack = True
        cfg.options.optimize = True
        cfg.options.release = True
        configure(cfg)

    if 'clang-msvc-debug' in variants:
        cfg.setenv('clang-msvc-debug')
        cfg.env.AR = clang_bin+'llvm-lib'
        cfg.env.CC = clang_bin+'clang-cl'
        cfg.env.CXX = clang_bin+'clang-cl'
        cfg.env.WINRC = winrc
        cfg.env.WINRCFLAGS = '-m32'
        cfg.env.LINK_CXX = clang_bin+'lld-link'
        cfg.env.CXXFLAGS = cfg.env.CFLAGS = ['-m32'] + clang_cxxflags
        cfg.env.LINKFLAGS = [
            '/libpath:/mnt/msvc/vc12/lib',
            '/libpath:/mnt/msvc/vc12/win_sdk/Lib/winv6.3/um/x86',
            #'/debug',
        ]
        cfg.options.clang_hack = True
        cfg.options.optimize = True
        cfg.options.release = False
        configure(cfg)


    if 'clang-msvc64' in variants:
        cfg.setenv('clang-msvc64')
        cfg.env.AR = clang_bin+'llvm-lib'
        cfg.env.CC = clang_bin+'clang-cl'
        cfg.env.CXX = clang_bin+'clang-cl'
        cfg.env.WINRC = winrc
        cfg.env.WINRCFLAGS = '-m64'
        cfg.env.LINK_CXX = clang_bin+'lld-link'
        cfg.env.CXXFLAGS = cfg.env.CFLAGS = clang_cxxflags
        cfg.env.LINKFLAGS = [
            '/opt:lldlto=1',
            '/libpath:/mnt/msvc/vc12/lib/amd64',
            '/libpath:/mnt/msvc/vc12/win_sdk/Lib/winv6.3/um/x64',
        ]
        cfg.options.clang_hack = True
        cfg.options.optimize = True
        cfg.options.release = True
        configure(cfg)

from waflib.Configure import ConfigurationContext
class my_configure_cls(ConfigurationContext):
    cmd = 'my_configure'
    fun = 'my_configure'

from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext

def init(ctx):
    for x in variants:
        for y in (BuildContext, CleanContext, InstallContext, UninstallContext, TestContext):
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
    # timer = Utils.Timer()
    # threads = []
    # count = [0]
    # line_lock = Utils.threading.Lock()
    # class sub_build(Utils.threading.Thread):
    #     def run(self):
    #         bld = self.bld = self.cls(top_dir=ctx.top_dir, out_dir=ctx.out_dir)
    #         bld.restore()
    #         bld.siblings = threads
    #         bld.count = count
    #         bld.line_lock = line_lock
    #         bld.timer = timer
    #         bld.logger = ctx.logger
    #         bld.load_envs()
    #         bld.targets = ctx.targets
    #         bld.recurse([bld.run_dir])
    #         bld.compile()

    # for x in variants:
    #     cls = type(Build.BuildContext)(x, (Build.BuildContext,), {'cmd': x, 'variant': x})
    #     f = sub_build()
    #     f.cls = cls
    #     threads.append(f)
    #     f.start()

    # for x in threads:
    #     x.join()

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


class package_ctx(Build.BuildContext):
    cmd = fun = 'package'

def package(ctx):
    _build_many(ctx, ['clang-msvc', 'clang-msvc-debug'])

    import zipfile, zlib
    def get_compr(t):
        # you can't change compression level, are python developers retarded?
        # yes, they are
        return zlib.compressobj(9, zlib.DEFLATED, -15, 9)
    zipfile._get_compressor = get_compr # for python3
    zlib.Z_DEFAULT_COMPRESSION = 9 # for python2, but worse

    def make_zip(ctx, variant, name_suff):
        ctx.variant = variant
        ctx.env = ctx.all_envs[variant]
        ctx.init_dirs()

        pdir = ctx.path.make_node('pkg')
        pdir.mkdir()
        zipn = pdir.make_node('neptools-windows-%s%s.zip' % (VERSION, name_suff))
        with zipfile.ZipFile(zipn.abspath(), 'w', zipfile.ZIP_DEFLATED) as zip:
            for s in ['COPYING', 'README.md', 'launcher.exe',
                      'neptools-server.dll', 'stcm-editor.exe']:
                zip.write(ctx.path.find_or_declare(s).abspath(), s)

    make_zip(ctx, 'clang-msvc', '')
    make_zip(ctx, 'clang-msvc-debug', '-debug')
