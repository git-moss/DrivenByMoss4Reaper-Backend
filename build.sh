export JAVA_HOME=~/java/jdk-17.0.5+8/

mkdir -p build
cmake -S "CMake" -B "build" -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --verbose
