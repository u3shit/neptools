#! /bin/bash

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 vm.opts run-tests.exe" >&2
    exit 1
fi

dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

temp_img=img.img
# should set:
# username: ssh login user
# tmp_dir: temp dir inside vm
# src_img: image that will be copied to $temp_img to use
# qemu_opts: additional qemu options (bash array)
#  should probably contain: -enable-kvm -nodefaults -m -cpu -smp -rtc
#  -drive (with file=$temp_img), -netdev user, -loadvm
# you have to modify this script if you don't want to use user networking
source "$1"

set -eEx
function cleanup() {
    "$dir/qmp" --path=qemu.sock quit
    exit 1
}
trap cleanup ERR

cp --reflink=always "$src_img" "$temp_img"

qemu-system-x86_64 \
    "${qemu_opts[@]}" \
    -display none -monitor none \
    -daemonize -qmp unix:qemu.sock,server,nowait

for i in $(seq 0 4); do
    port=$(python -c 'import socket; s=socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()')
    fwd="$("$dir/qmp" --path=qemu.sock human-monitor-command \
                      --command-line="hostfwd_add tcp:127.0.0.1:$port-:22")"
    if [[ -z $fwd ]]; then break; fi
done

scp -o StrictHostKeyChecking=no -P $port "$2" "$username@localhost:$tmp_dir/"
sleep 1 | ssh -o StrictHostKeyChecking=no -p $port $username@localhost \
              "cd $tmp_dir && run-tests"

"$dir/qmp" --path=qemu.sock quit
