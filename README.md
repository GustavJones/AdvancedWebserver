# Advanced Webserver

1. Description
2. Usage
3. Building

## Description

This is a simple C++ project to deploy an HTTPS server mostly from scratch
with options to configure URI relocation and different URI handling modes.

## Usage

For now I have yet to configure the CMake --install script,
so for now the utility can be run from source in the build directory.

Note that the server will require a valid `.crt` and `.key` file
in the running directory for SSL purposes. This will change in the future.

Both tools can be run with `--help` flag for more information.

Run the `build/src/Server/AdvancedWebserver` file to start the server.
Run the `build/src/ConfigureTool/AdvancedWebserver-ConfigureTool` to configure the server.

## Building

#### Prerequisites

Install OpenSSL development libraries.

#### Building

```bash
# Clone the source code and the submodules
git clone --recursive https://github.com/GustavJones/AdvancedWebserver.git
cd AdvancedWebserver

# Configure the project to the build directory
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF

# Build the source code to the output build directory
cmake --build build -j `nproc` # Run nproc first and replace result here
```
