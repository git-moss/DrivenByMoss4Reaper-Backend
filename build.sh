export JAVA_HOME=~/java/jdk-21.0.4+7/

mkdir -p build
cmake -S "CMake" -B "build" -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --verbose
