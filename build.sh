export JAVA_HOME=/home/mos/java/jdk-16.0.2+7/

mkdir -p build
cmake -S "CMake" -B "build"
cmake --build "build" --config Release

mv build/libreaper_drivenbymoss.so build/reaper_drivenbymoss.so