export JAVA_HOME=/home/mos/java/jdk-17.0.1+12/

mkdir -p build
cmake -S "CMake" -B "build" -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --verbose
