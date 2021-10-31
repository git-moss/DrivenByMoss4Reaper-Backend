mkdir build
cmake -S "CMake" -B "build" -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --config Release
