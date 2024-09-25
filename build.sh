# Change directory to .sh file location
cd $(dirname $0)

# Configure project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DBUILD_SHARED_LIBS=OFF

# Build project
cmake --build build -j
