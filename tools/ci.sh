#! /bin/sh

function einfo()
{
    echo -e " \033[32m*\033[0m $@" >&2
}

function ewarn()
{
    echo -e " \033[33m*\033[0m $@" >&2
}

function eerror()
{
    echo -e " \033[31m*\033[0m $@" >&2
}

trap "exit 1" ERR

if [[ $# != 2 ]]; then
    eerror "Usage: $0 compiler mode"
    exit 1
fi
compiler="$1"
mode="$2"

if [[ $mode != rel && $mode != debug ]]; then
    eerror "Invalid mode $mode"
    exit 1
fi

cd "$(dirname "${BASH_SOURCE[0]}")/.."

function before() { :; }
function after() { :; }

function load()
{
    if [[ -f ~/.neptools_ci_conf."$1".sh ]]; then
        source ~/.neptools_ci_conf."$1".sh
    else
        source tools/ci_conf."$1".sh
    fi
}

function run()
{
    einfo "$@"
    "$@" || exit 1
}

load default
load "$compiler"

if [[ $mode = rel ]]; then
    mode_arg=--release
else
    mode_arg=--optimize-ext
fi

before
run ./waf --color=yes configure "${config_opts[@]}" "$mode_arg" --all-bundle
run ./waf --color=yes build test "${build_opts[@]}" --skip-run-tests
if [[ -f build/stcm-editor && $mode = rel ]]; then
    run strip --strip-unneeded -R .comment -R .GCC.command.line build/stcm-editor
fi

ret=0
for t in "${tests[@]}"; do
    einfo "$t"
    eval "$t" || ret=1
done
after
exit $ret
