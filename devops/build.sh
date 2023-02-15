cd ..
mkdir build
cd build/
cmake .. -DVCPKG_ROOT=/home/ivan/vcpkg
make
cd ..
rm -rf build
