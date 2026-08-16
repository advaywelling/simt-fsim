# simt-fsim
A functional SIMT simulator that executes GPU kernel IR (my bad custom ISA, PTX, SPIR-V...), reporting SIMD utilization and memory coalescing

# What works right now
- SIMD utilization and coalescing works (you can only test if you write kernels in my IR, PTX and SPIR-V are FARRR from being supported)
- ~~Forward control flow only - no loops yet because that's hard but I am working on it right now~~
- Loops work :))))
- IPDOM reconvergence! You don't have to handfeed branch reconvergence PCs into instructions, it's handled behind the scenes :)
- Predication with guards for basic control flow

# To build
cmake -B build

cmake --build build
# To run
For all tests

./build/sim

For specific test (alu/branch/lw/cfg)

./build/sim -placename

eg.
./build/sim branch

The tests are self-checking - the exit code is non-zero if any check fails.
You can also run them through ctest:

ctest --test-dir build --output-on-failure
