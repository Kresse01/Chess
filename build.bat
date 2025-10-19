cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=C:/libs-msys
cmake --build build --config Debug
cmake --install build --config Debug