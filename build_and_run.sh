cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
make -C build
echo ""
./build/roofline-demo