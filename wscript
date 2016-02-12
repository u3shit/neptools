# -*- mode: python -*-

# the following two variables are used by the target "waf dist"
VERSION='0.2.2'
APPNAME='stcm-editor'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx boost')
    opt.add_option('--msvc-hack', action='store_true', default=False,
                   help='Read COMPILE.md...')
    opt.add_option('--release', action='store_true', default=False,
                   help='Enable some flags for release builds')

def configure(cfg):
    if cfg.options.msvc_hack:
        cfg.env.DEST_OS = 'win32'
        if 'LINK_CXX' in cfg.environ:
            cfg.env['LINK_CXX'] = cfg.environ['LINK_CXX']
        cfg.load('msvc', funs='no_autodetect')

        from waflib.Tools.compiler_cxx import cxx_compiler
        from waflib import Utils
        cxx_compiler[Utils.unversioned_sys_platform()] = ['msvc']

    cfg.load('compiler_cxx boost clang_compilation_database')

    if cfg.env['COMPILER_CXX'] == 'msvc':
        cfg.env.append_value('CXXFLAGS', ['/EHsc', '/Za', '/MD'])
        if cfg.options.release:
            cfg.env.prepend_value('CXXFLAGS', [
                '/O2', '/Gv', '/GL', '/Gw', '/Gy'])
            cfg.env.prepend_value('LINKFLAGS', ['/LTCG', '/OPT:REF', '/OPT:ICF'])
        cfg.check_boost()
    else:
        cfg.check_cxx(cxxflags='-std=c++14')
        cfg.env.append_value('CXXFLAGS', ['-std=c++14'])
        cfg.env.append_value('CXXFLAGS', cfg.filter_flags([
            '-fcolor-diagnostics', '-Wall', '-Wextra', '-pedantic',
            '-Wno-parentheses']))
        cfg.check_boost(lib='filesystem system')

        if cfg.options.release:
            opt=['-Ofast', '-flto', '-fno-fat-lto-objects',
                 '-fomit-frame-pointer']
            cfg.check_cxx(cxxflags=opt)

            cfg.env.prepend_value('CXXFLAGS', opt)
            cfg.env.prepend_value('LINKFLAGS', opt + ['-Wl,-O1'])

    if cfg.options.release:
        cfg.define('NDEBUG', 1)
    if cfg.env.DEST_OS == 'win32':
        cfg.define('UNICODE', 1)
        cfg.define('_UNICODE', 1)

def build(bld):
    src = [
        'src/buffer.cpp',
        'src/context.cpp',
        'src/dynamic_struct.cpp',
        'src/dumpable.cpp',
        'src/gbnl.cpp',
        'src/item.cpp',
        'src/main.cpp',
        'src/raw_item.cpp',
        'src/utils.cpp',
        'src/cl3/file.cpp',
        'src/cl3/file_collection.cpp',
        'src/cl3/header.cpp',
        'src/cl3/sections.cpp',
        'src/stcm/collection_link.cpp',
        'src/stcm/data.cpp',
        'src/stcm/exports.cpp',
        'src/stcm/file.cpp',
        'src/stcm/gbnl.cpp',
        'src/stcm/header.cpp',
        'src/stcm/instruction.cpp',
    ]
    bld.program(source = src,
                uselib = 'BOOST',
                target = APPNAME)


from waflib.Configure import conf
@conf
def filter_flags(cfg, flags):
    ret = []

    for flag in flags:
        try:
            cfg.check_cxx(cxxflags=flag)
            ret.append(flag)
        except:
            pass

    return ret
