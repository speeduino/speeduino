git submodule update --init --recursive
cd googletest/googletest
mkdir build
cd build
cmake ../
make
cd ../../..
ln -s googletest/googletest/lib ./
ln -s googletest/googletest/include/gtest ./include/


