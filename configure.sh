mkdir build
cd build
echo $DYNAMORIO_DIR
cmake -DDynamoRIO_DIR=$DYNAMORIO_DIR/cmake ../src
make
cd ../


