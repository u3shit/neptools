# -*- mode: python -*-

# libshit 'config'
APPNAME='neptools'
BOOST_LIBS = ['system', 'filesystem']
DEFAULT_LUA = 'ljx'

def options(opt):
    opt.recurse('libshit', name='options')

def configure(cfg):
    cfg.recurse('libshit', name='configure', once=False)

def build_common(bld):
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
    if not bld.env.WITHOUT_LUA:
        src += [
            'src/txt_serializable.cpp',
            'src/format/builder.lua',
        ]

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

def build(bld):
    build_common(bld)

    bld.program(source = 'src/programs/stcm-editor.cpp src/programs/stcm-editor.rc',
                includes = 'src', # for version.hpp
                uselib = 'NEPTOOLS',
                use = 'common common-stsc',
                target = 'stcm-editor')

    if bld.env.DEST_OS == 'win32':
        # technically launcher can be compiled for 64bits, but it makes no sense
        ld = ['/nodefaultlib', '/entry:start', '/subsystem:windows', '/FIXED',
              '/NXCOMPAT:NO', '/IGNORE:4254']
        bld.program(source = 'src/programs/launcher.c src/programs/launcher.rc',
                    includes = 'src', # for version.hpp
                    target = 'launcher',
                    cflags = '-O1 -Gs9999999',
                    uselib = 'KERNEL32 SHELL32 USER32 NEPTOOLS',
                    linkflags = ld)

        # server.dll has no implib (doesn't export anything)
        from waflib.TaskGen import taskgen_method
        @taskgen_method
        def apply_implib(self):
            if getattr(self, 'defs', None) and self.env.DEST_BINFMT == 'pe':
                node = self.path.find_resource(self.defs)
                if not node:
                    raise Errors.WafError('invalid def file %r' % self.defs)

                self.env.append_value('LINKFLAGS', '/def:%s' % node.path_from(self.bld.bldnode))
                self.link_task.dep_nodes.append(node)

        src_inject = [
            'src/injected/cpk.cpp',
            'src/injected/hook.cpp',
            'src/programs/server.cpp',
            'src/programs/server.rc',
        ]
        bld.shlib(source = src_inject,
                  includes = 'src', # for version.hpp
                  target = 'neptools-server',
                  use    = 'common',
                  uselib = 'SHELL32 USER32 NEPTOOLS',
                  defs   = 'src/programs/server.def')

def test(bld):
    build_common(bld)

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

    src = [
        'libshit/test/main.cpp',
        'test/pattern.cpp',
        'test/sink.cpp',
        'test/container/ordered_map.cpp',
        'test/container/parent_list.cpp',
    ]
    bld.program(source   = src,
                includes = 'src libshit/ext/catch/include',
                uselib   = 'NEPTOOLS',
                use      = 'common libshit-tests',
                target   = 'run-tests')
