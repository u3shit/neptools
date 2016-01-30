# -*- mode: python -*-

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='editor'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
    opt.load('compiler_cxx boost')

def configure(cfg):
    cfg.load('compiler_cxx boost clang_compilation_database')

    cfg.check_cxx(cxxflags='-std=c++14')
    cfg.env.append_value('CXXFLAGS', ['-std=c++14'])
    cfg.env.append_value('CXXFLAGS', cfg.filter_flags([
        '-fcolor-diagnostics', '-Wall', '-Wextra', '-pedantic', '-Wno-parentheses']))

    cfg.check_boost(lib='')

def build(bld):
    src = [
        'src/buffer.cpp',
        'src/context.cpp',
        'src/item.cpp',
        'src/main.cpp',
        'src/raw_item.cpp',
        'src/stcm/exports.cpp',
        'src/stcm/file.cpp',
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
