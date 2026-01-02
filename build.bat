cmake -S . -B build ^
  -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe ^
  -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ^
  -DCMAKE_INSTALL_PREFIX=C:/libs-msys
cmake --build build --config Debug
cmake --install build --config Debug