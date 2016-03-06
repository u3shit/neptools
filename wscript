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

def build_common(bld):
    bld(features = 'subst',
        source = 'src/version.hpp.in',
        target = 'src/version.hpp',
        VERSION = VERSION)

    src = [
        'src/except.cpp',
        'src/dumpable.cpp',
        'src/pattern.cpp',
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

def build(bld):
    build_common(bld)

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
                  use    = 'common',
                  uselib = 'BOOST USER32')

from waflib.Build import BuildContext
class TestContext(BuildContext):
    cmd = 'test'
    fun = 'test'

def test(bld):
    build_common(bld)

    # feature fail_cxx: expect compilation failure
    import sys
    from waflib import Logs
    from waflib.Task import Task
    from waflib.TaskGen import extension, feature
    from waflib.Tools import c_preproc
    @extension('.cpp')
    def fail_cxx_ext(self, node):
        if 'fail_cxx' in self.features:
            return self.create_compiled_task('fail_cxx', node)
        else:
            return self.create_compiled_task('cxx', node)

    class fail_cxx(Task):
        #run_str = '${CXX} ${ARCH_ST:ARCH} ${CXXFLAGS} ${CPPFLAGS} ${FRAMEWORKPATH_ST:FRAMEWORKPATH} ${CPPPATH_ST:INCPATHS} ${DEFINES_ST:DEFINES} ${CXX_SRC_F}${SRC} ${CXX_TGT_F}${TGT[0].abspath()}'
        def run(tsk):
            env = tsk.env
            gen = tsk.generator
            bld = gen.bld
            cwdx = getattr(bld, 'cwdx', bld.bldnode) # TODO single cwd value in waf 1.9
            wd = getattr(tsk, 'cwd', None)
            def to_list(xx):
                if isinstance(xx, str): return [xx]
                return xx
            tsk.last_cmd = lst = []
            lst.extend(to_list(env['CXX']))
            lst.extend(tsk.colon('ARCH_ST', 'ARCH'))
            lst.extend(to_list(env['CXXFLAGS']))
            lst.extend(to_list(env['CPPFLAGS']))
            lst.extend(tsk.colon('FRAMEWORKPATH_ST', 'FRAMEWORKPATH'))
            lst.extend(tsk.colon('CPPPATH_ST', 'INCPATHS'))
            lst.extend(tsk.colon('DEFINES_ST', 'DEFINES'))
            lst.extend(to_list(env['CXX_SRC_F']))
            lst.extend([a.path_from(cwdx) for a in tsk.inputs])
            lst.extend(to_list(env['CXX_TGT_F']))
            lst.append(tsk.outputs[0].abspath())
            lst = [x for x in lst if x]
            try:
                (out,err) = bld.cmd_and_log(
                    lst, cwd=bld.variant_dir, env=env.env or None,
                    quiet=0, output=0)
                Logs.error("%s compiled successfully, but it shouldn't" % tsk.inputs[0])
                Logs.info(out, extra={'stream':sys.stdout, 'c1': ''})
                Logs.info(err, extra={'stream':sys.stderr, 'c1': ''})
                return -1
            except Exception as e:
                # create output to silence missing file errors
                open(tsk.outputs[0].abspath(), 'w').close()
                return 0

        vars    = ['CXXDEPS'] # unused variable to depend on, just in case
        ext_in  = ['.h'] # set the build order easily by using ext_out=['.h']
        scan    = c_preproc.scan

    # test pattern parser
    bld(features='cxx',
        source='test/pattern_fail.cpp',
        uselib='BOOST',
        includes='src',
        defines=['TEST_PATTERN="b2 ff"_pattern'])
    bld(features='cxx fail_cxx',
        source='test/pattern_fail.cpp',
        uselib='BOOST',
        includes='src',
        defines=['TEST_PATTERN="bz ff"_pattern'])

    src = [
        'test/main.cpp',
        'test/pattern.cpp',
    ]
    bld.program(source   = src,
                includes = 'src ext/catch/include',
                uselib   = 'BOOST',
                use      = 'common',
                target   = 'run-tests')
    bld.add_post_fun(lambda ctx:
        ctx.exec_command([
            bld.bldnode.find_or_declare('run-tests').abspath(),
            '--use-colour', 'yes']) == 0 or ctx.fatal('Test failure')
    )

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
