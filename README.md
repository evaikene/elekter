# elekter

This command-line tool is used to process CSV files with power consumption
data downloaded from https://elering.ee.

## Building

The tool can be built on Linux or MacOS and requires the following dependencies:

* **CMake**
* **Qt (6.8)** - core, network and sql components are used
* **libfmt** - for formatting output

Create a build directory and run the following commands:

```sh
mkdir bld
cd bld
cmake ..
make
```

Dependencies can be installed automatically using `vcpkg`:

```sh
cmake -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake ..
```

## Using

Download CSV power consumption files from https://elering.ee and then analyze
them with this tool.

Print out total consumption and calculate the cost using Nord Pool hourly prices:

```sh
elekter Tunnitarbimise\ andmed.csv -p -k
```
