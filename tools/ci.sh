#! /bin/bash

export TERM=xterm

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

if [[ $mode != rel && $mode != rel-test && $mode != debug ]]; then
    eerror "Invalid mode $mode"
    exit 1
fi

cd "$(dirname "${BASH_SOURCE[0]}")/.."

function before_build() { :; }
function after_build() { :; }
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

before_build
build
after_build

ret=0
if [[ $mode != rel ]]; then
    for t in "${tests[@]}"; do
        einfo "$t"
        eval "$t" || ret=1
    done
fi
after
exit $ret
