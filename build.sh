export JAVA_HOME=/home/mos/java/jdk-16.0.2+7/

mkdir -p build
cmake -S "CMake" -B "build" -D CMAKE_BUILD_TYPE=Release
cmake --build "build" --verbose
