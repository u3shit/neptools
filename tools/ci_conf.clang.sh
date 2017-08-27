run export CC=clang
run export CXX=clang++
run export CXXFLAGS=-stdlib=libc++
run export LINKFLAGS='-stdlib=libc++ -fuse-ld=gold -static-libstdc++ -Wl,--as-needed -lpthread -Wl,--no-as-needed'
