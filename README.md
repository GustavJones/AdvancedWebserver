# Advanced Webserver

1. Description
2. Modes
3. Usage
4. Building

## Description

This is a simple C++ project to deploy an HTTPS server mostly from scratch
with options to configure URI relocation and different URI handling modes.
Note this server only supports Linux environments at the moment.

## Modes

The server includes four different request handling modes including:

- file_io
- folder_io
- executable
- cascading_executable

#### File_IO

Let you define a static location on your disk to map to a specific URI from a request.
Example:

Setup: `AdvancedWebserver-Configure-Tool / file_io /opt/Website/index.html text/html`

`https://127.0.0.1:8081/`: `/` is mapped to `/opt/Website/index.html` file on my hard drive and returns the file.

#### Folder_IO

Let you define a dynamic location on your disk to map to a range of URI's from a request.
Example:

Setup: `AdvancedWebserver-Configure-Tool / folder_io /opt/Website/ text/html`

`https://127.0.0.1:8081/`: `/` is mapped to the `/opt/Website/` directory and will return the corresponding files in the directory.
e.g.

- `https://127.0.0.1:8081/index.html` -> `/opt/Website/index.html`
- `https://127.0.0.1:8081/home.html` -> `/opt/Website/home.html`

#### Executable

Let you define a static path to a program to map to a specific URI from a request.
This program will recieve a file path as a command line argument that will contain a
file with an HTTP request from the server. The program will then do some calculations defined
by the author and return an HTTP response to the same file provided by the argument.
Example:

Setup: `AdvancedWebserver-Configure-Tool /run executable /opt/Website/RunProgram`

`https://127.0.0.1:8081/run` -> `/opt/Website/RunProgram $FILE_PATH`

#### Cascading_Executable

Let you define a static path to a program to map to a range of URI's from a request.
This program will recieve a file path as a command line argument that will contain a
file with an HTTP request from the server. The program will then do some calculations defined
by the author and return an HTTP response to the same file provided by the argument.
Example:

Setup: `AdvancedWebserver-Configure-Tool /run cascading_executable /opt/Website/RunProgram`

`https://127.0.0.1:8081/run/arg1` -> `/opt/Website/RunProgram $FILE_PATH`
`https://127.0.0.1:8081/run/arg2` -> `/opt/Website/RunProgram $FILE_PATH`
`https://127.0.0.1:8081/run/arg3` -> `/opt/Website/RunProgram $FILE_PATH`

## Usage

Note that a valid SSL certificate and key must be provided
to the server through command line arguments

Both tools can be run with `--help` flag for more information.

Run the `build/src/Server/AdvancedWebserver` file to start the server.
Run the `build/src/ConfigureTool/AdvancedWebserver-Configure-Tool` to configure the server.

If the `sudo cmake --install build` command has been run, the programs can be accessed
by just typing `AdvancedWebserver` and `AdvancedWebserver-Configure-Tool` respectively to launch the programs

## Building

#### Prerequisites

Install OpenSSL Static development libraries.
(On some Linux distros the development package doesn't include the static libraries
and will require you to manually compile OpenSSL libraries statically)

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

#### Install

After the build you can run `sudo cmake --install build` and enter your password to install the AdvancedWebserver and AdvancedWebserver-Configure-Tool to /usr/local/bin/ directory
