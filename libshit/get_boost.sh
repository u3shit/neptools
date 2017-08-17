#! /bin/sh

TGT=ext/boost
DL=https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2
BZ2_FILE=ext/boost.tar.bz2
SHA1=9f1dd4fa364a3e3156a77dc17aa562ef06404ff6

set -e
if [[ -e $TGT ]]; then
    echo "$TGT exists, refusing to continue" >&2
    exit 1
fi

wget "$DL" -O "$BZ2_FILE"
echo "$SHA1 *$BZ2_FILE" | sha1sum -c

mkdir "$TGT"
tar -xf "$BZ2_FILE" -C "$TGT" --strip-components 1

echo "Successfully unpacked boost"
