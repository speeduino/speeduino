export CURDIR=$(pwd)
git submodule update --init --recursive
cd googletest/googletest
mkdir build
cd build
cmake ../
make
ln -sf $CURDIR/googletest/googletest/build/lib $CURDIR/lib
ln -sf $CURDIR/googletest/googletest/include/gtest/ $CURDIR/include/gtest


