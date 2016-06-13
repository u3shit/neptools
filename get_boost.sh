#! /bin/sh

TGT=ext/boost
DL=https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.bz2
BZ2_FILE=ext/boost.tar.bz2
SHA256=a547bd06c2fd9a71ba1d169d9cf0339da7ebf4753849a8f7d6fdb8feee99b640

set -e
if [[ -e $TGT ]]; then
    echo "$TGT exists, refusing to continue" >&2
    exit 1
fi

#wget "$DL" -O "$BZ2_FILE"
echo "$SHA256 *$BZ2_FILE" | sha256sum -c

mkdir "$TGT"
tar -xf "$BZ2_FILE" -C "$TGT" --strip-components 1

echo "Successfully unpacked boost"
