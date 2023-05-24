cmake -S . -B ./build
cd ./build
make
mv ./CP_Project ..
cd ..
./CP_Project ./test/test1.c