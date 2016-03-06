# -*- mode: python -*-

# the following two variables are used by the target "waf dist"
APPNAME='stcm-editor'

import subprocess
try:
    VERSION = subprocess.check_output(
        ['git', 'describe', '--tags', '--always'],
        stderr = subprocess.PIPE,
        universal_newlines = True).strip('\n').lstrip('v')
except:
    VERSION = '0.3.0'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

try:
    with open('wscript_user.py', 'r') as f:
        exec(compile(f.read(), 'wscript_user.py', 'exec'))
except IOError:
    pass

def options(opt):
    opt.load('compiler_c compiler_cxx boost')
    opt.add_option('--clang-hack', action='store_true', default=False,
                   help='Read COMPILE.md...')
    opt.add_option('--release', action='store_true', default=False,
                   help='Enable some flags for release builds')

def configure(cfg):
    if cfg.options.clang_hack:
        cfg.find_program('clang-cl', var='CC')
        cfg.find_program('clang-cl', var='CXX')
        cfg.find_program('lld-link', var='LINK_CXX')
        cfg.find_program('llvm-lib', var='AR')

        cfg.load('msvc', funs='no_autodetect')
        from waflib.Tools.compiler_cxx import cxx_compiler
        from waflib import Utils
        cxx_compiler[Utils.unversioned_sys_platform()] = ['msvc']
        from waflib.Tools.compiler_c import c_compiler
        c_compiler[Utils.unversioned_sys_platform()] = ['msvc']

    cfg.load('compiler_cxx boost clang_compilation_database')

    flags = cfg.filter_flags(['CFLAGS', 'CXXFLAGS'], [
        # error on unknown arguments, including unknown options that turns
        # unknown argument warnings into error. どうして？
        '-Werror=unknown-warning-option',
        '-Werror=ignored-optimization-argument',
        '-Werror=unknown-argument',

        '-fcolor-diagnostics', '-fdiagnostics-show-option',
        '-Wall', '-Wextra', '-pedantic', '-Wno-parentheses',
        '-Wno-gnu-string-literal-operator-template'])

    if cfg.env['COMPILER_CXX'] == 'msvc':
        cfg.define('_CRT_SECURE_NO_WARNINGS', 1)
        cfg.env.append_value('CXXFLAGS', [
            '-Xclang', '-std=c++14',
            '-Xclang', '-fdiagnostics-format', '-Xclang', 'clang',
            '-EHsc', '-MD'])
        cfg.env.prepend_value('CFLAGS', '/Gs9999999')

        if cfg.options.release:
            cfg.env.prepend_value('CFLAGS', ['/O1', '/GS-'])
            cfg.env.prepend_value('CXXFLAGS', [
                '-O2', '-Xclang', '-emit-llvm-bc'])
            cfg.env.prepend_value('LINKFLAGS', ['/OPT:REF', '/OPT:ICF'])
    else:
        cfg.check_cxx(cxxflags='-std=c++14')
        cfg.env.append_value('CXXFLAGS', ['-std=c++14'])

        if cfg.options.release:
            cfg.filter_flags(['CXXFLAGS', 'LINKFLAGS'], [
                '-Ofast', '-flto', '-fno-fat-lto-objects',
                 '-fomit-frame-pointer'])

            cfg.env.append_value('LINKFLAGS', '-Wl,-O1')

    def chkdef(cfg, defn):
        return cfg.check_cxx(fragment='''
#ifndef %s
#error err
#endif
int main() { return 0; }
''' % defn,
                             msg='Checking for '+defn,
                             features='cxx',
                             mandatory=False)

    if chkdef(cfg, '_WIN64'):
        cfg.env.DEST_OS = 'win64'
    elif chkdef(cfg, '_WIN32'):
        cfg.env.DEST_OS = 'win32'

    fs_check = '''
#include <experimental/filesystem>
int main()
{
    std::experimental::filesystem::exists("foo");
    return 0;
}
'''
    if cfg.check_cxx(header_name='experimental/filesystem',
                     fragment=fs_check, mandatory=False):
        cfg.check_boost()
    else:
        cfg.check_boost(lib='filesystem system')

    if cfg.options.release:
        cfg.define('NDEBUG', 1)
    if cfg.env.DEST_OS == 'win32' or cfg.env.DEST_OS == 'win64':
        cfg.define('WINDOWS', 1)
        cfg.define('UNICODE', 1)
        cfg.define('_UNICODE', 1)

        cfg.check_cxx(lib='kernel32')
        cfg.check_cxx(lib='shell32')
        cfg.check_cxx(lib='user32')

def build(bld):
    bld(features = 'subst',
        source = 'src/version.hpp.in',
        target = 'src/version.hpp',
        VERSION = VERSION)

    src = [
        'src/except.cpp',
        'src/dumpable.cpp',
        'src/source.cpp',
        'src/utils.cpp',
        'src/format/context.cpp',
        'src/format/gbnl.cpp',
        'src/format/item.cpp',
        'src/format/raw_item.cpp',
        'src/format/cl3.cpp',
        'src/format/stcm/collection_link.cpp',
        'src/format/stcm/data.cpp',
        'src/format/stcm/exports.cpp',
        'src/format/stcm/file.cpp',
        'src/format/stcm/gbnl.cpp',
        'src/format/stcm/header.cpp',
        'src/format/stcm/instruction.cpp',
    ]

    bld.stlib(source = src,
              uselib = 'BOOST',
              target = 'common')

    bld.program(source = 'src/programs/stcm-editor.cpp',
                includes = 'src', # for version.hpp
                uselib = 'BOOST',
                use = 'common',
                target = APPNAME)

    if bld.env.DEST_OS == 'win32':
        # technically launcher can be compiled for 64bits, but it makes no sense
        ld = ['/nodefaultlib', '/entry:start', '/subsystem:windows', '/FIXED',
              '/NXCOMPAT:NO', '/IGNORE:4254']
        bld.program(source = 'src/programs/launcher.c',
                    target = 'launcher',
                    uselib = 'KERNEL32 SHELL32 USER32',
                    linkflags = ld)

        # server.dll has no implib (doesn't export anything)
        from waflib.TaskGen import taskgen_method
        @taskgen_method
        def apply_implib(self):
            pass

        src_inject = [
            'src/injected/cpk.cpp',
            'src/injected/hook.cpp',
            'src/programs/server.cpp',
        ]
        bld.shlib(source = src_inject,
                  target = 'server',
                  uselib = 'BOOST USER32')

from waflib.Configure import conf
@conf
def filter_flags(cfg, vars, flags):
    ret = []

    for flag in flags:
        try:
            # gcc ignores unknown -Wno-foo flags but not -Wfoo, but warns if
            # there are other warnings. with clang, just depend on
            # -Werror=ignored-*-option, -Werror=unknown-*-option
            testflag = flag
            if flag[0:5] == '-Wno-':
                testflag = '-W'+flag[5:]
            cfg.check_cxx(cxxflags=[testflag], features='cxx',
                          msg='Checking for compiler flag '+testflag)
            ret.append(flag)
            for var in vars:
                cfg.env.append_value(var, flag)
        except:
            pass

    return ret
