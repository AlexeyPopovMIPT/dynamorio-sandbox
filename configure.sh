mkdir build
cd build
cmake -DDynamoRIO_DIR=$DYNAMORIO_DIR/cmake ../src
make
cd ../
