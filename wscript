# -*- mode: python -*-

# libshit 'config'
APPNAME='neptools'
BOOST_LIBS = ['system', 'filesystem']
DEFAULT_LUA = 'ljx'

def options(opt):
    opt.recurse('libshit', name='options')

def configure(cfg):
    cfg.recurse('libshit', name='configure', once=False)

    if cfg.env.DEST_OS == 'vita':
        cfg.check_cxx(lib='taihen_stub', uselib_store='TAIHEN')

def build(bld):
    bld.recurse('libshit')

    src = [
        'src/dumpable.cpp',
        'src/endian.cpp',
        'src/low_io.cpp',
        'src/open.cpp',
        'src/pattern.cpp',
        'src/sink.cpp',
        'src/source.cpp',
        'src/utils.cpp',
        'src/format/cl3.cpp',
        'src/format/context.cpp',
        'src/format/cstring_item.cpp',
        'src/format/eof_item.cpp',
        'src/format/gbnl.cpp',
        'src/format/item.cpp',
        'src/format/primitive_item.cpp',
        'src/format/raw_item.cpp',
        'src/format/stcm/collection_link.cpp',
        'src/format/stcm/data.cpp',
        'src/format/stcm/exports.cpp',
        'src/format/stcm/file.cpp',
        'src/format/stcm/gbnl.cpp',
        'src/format/stcm/header.cpp',
        'src/format/stcm/instruction.cpp',
        'src/format/stcm/string_data.cpp',
    ]
    if bld.env.WITH_LUA:
        src += [
            'src/txt_serializable.cpp',
            'src/format/builder.lua',
        ]
    if bld.env.WITH_TESTS:
        src += [ 'test/pattern.cpp' ]

    bld.objects(source   = src,
                uselib   = 'NEPTOOLS',
                use      = 'libshit boost_system boost_filesystem',
                includes = 'src',
                target   = 'common')

    src = [
        'src/format/stsc/file.cpp',
        'src/format/stsc/header.cpp',
        'src/format/stsc/instruction.cpp',
    ]
    bld.objects(source   = src,
                uselib   = 'NEPTOOLS',
                use      = 'libshit boost_system boost_filesystem',
                includes = 'src',
                target   = 'common-stsc')

    if bld.env.DEST_OS == 'vita':
        bld.program(source   = 'src/vita_plugin/plugin.cpp',
                    includes = 'src', # for version.hpp
                    ldflags  = '-lSceAppMgr_stub',
                    uselib   = 'NEPTOOLS TAIHEN',
                    use      = 'common',
                    target   = 'vita_plugin')
    else:
        bld.program(source = ['src/programs/stcm-editor.cpp',
                              'src/programs/stcm-editor.rc'],
                    includes = 'src', # for version.hpp
                    uselib = 'NEPTOOLS',
                    use = 'common common-stsc',
                    target = 'stcm-editor')

    if bld.env.DEST_OS == 'win32' and bld.env.DEST_CPU == 'x86':
        # technically launcher can be compiled for 64bits, but it makes no sense
        ld = ['-Wl,/nodefaultlib', '-Wl,/entry:start', '-Wl,/subsystem:windows',
              '-Wl,/FIXED', '-Wl,/NXCOMPAT:NO', '-Wl,/IGNORE:4254']
        bld.program(source = 'src/programs/launcher.c src/programs/launcher.rc',
                    includes = 'src', # for version.hpp
                    target = 'launcher',
                    cflags = '-Os -mstack-probe-size=999999999 -fno-stack-protector',
                    uselib = 'KERNEL32 SHELL32 USER32 NEPTOOLS',
                    linkflags = ld)

        src_inject = [
            'src/windows_server/cpk.cpp',
            'src/windows_server/hook.cpp',
            'src/windows_server/server.cpp',
            'src/windows_server/server.rc',
        ]
        bld.shlib(source = src_inject,
                  includes = 'src', # for version.hpp
                  target = 'neptools-server',
                  use    = 'common',
                  uselib = 'SHELL32 USER32 NEPTOOLS',
                  defs   = 'src/windows_server/server.def')

    if bld.env.WITH_TESTS:
        # test pattern parser
        bld(features='cxx',
            source='test/pattern_fail.cpp',
            uselib='BOOST NEPTOOLS',
            use='boost_system boost_filesystem libshit',
            includes='src',
            defines=['TEST_PATTERN="b2 ff"'])
        bld(features='cxx fail_cxx',
            source='test/pattern_fail.cpp',
            uselib='BOOST NEPTOOLS',
            use='boost_system boost_filesystem libshit',
            includes='src',
            defines=['TEST_PATTERN="bz ff"'])
