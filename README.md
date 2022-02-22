# compile for arm
mkdir build
cmake -DARM_BOARD=true ..
make
make install
