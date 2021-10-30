export JAVA_HOME=/Library/Java/JavaVirtualMachines/temurin-16.jdk/Contents/Home

mkdir -p build
cmake -S "CMake" -B "build" -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --verbose
