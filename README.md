# simt-fsim
A functional SIMT simulator that executes GPU kernel IR (my bad custom ISA, PTX, SPIR-V...), reporting SIMD utilization and memory coalescing - not cycle-accurate

# To build
cmake -B build
cmake --build build
# To run
For all tests
./build/src
For specific test (example "branch")
./build/src branch
