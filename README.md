# simt-fsim
A functional SIMT simulator that executes GPU kernels through a custom IR, reporting SIMD utilization and memory coalescing

# What works right now
- SIMD utilization and coalescing works (you can only test if you write kernels in my IR sorry)
- ~~Forward control flow only - no loops yet because that's hard but I am working on it right now~~
- Loops work!
- IPDOM reconvergence - you don't have to handfeed branch reconvergence PCs into instructions, it's handled behind the scenes
- Predication with guards for basic control flow


# How it works
The interesting part is figuring out where diverged lanes come back together.

When a branch splits the wave, some lanes go one way and some go the other. You have to run 
them separately, but you want them back on the same instruction ASAP - every instruction you
run at half occupancy is half your SIMD throughput cooked. So the question is
"where do these lanes definitely meet again?"

1. Split the program into basic blocks and build a CFG
2. Compute post-dominators - the blocks that every path from here has to go through
3. Take the immediate post-dominator of each block. That's the closest one, so it's the
   earliest point the lanes can possibly reconverge
4. At a divergent branch, push a JOIN entry holding the full mask plus one entry per path
   onto a SIMT stack, all tagged with that reconvergence PC. Pop the top and run it. When the
   PC reaches the reconvergence point, pop the next path. When only the JOIN is left, popping
   it puts everyone back together

Uniform branches (every active lane agrees on where to go) skip all of that and just set the
PC. That simplifies loops too since a backedge is unconditional, so it never splits
anything, it just jumps.

# Numbers
Here are some of the numbers I've gotten till now!
Configuration - 32 lanes/threads per wave, each global memory segment is 32 words
I ran a few basic kernels and loops from `./build/sim kernels` and `./build/sim loop`

| Kernel | SIMD utilization | Coalescing |
|---|---|---|
| Vector add, consecutive words | 100% | 100% |
| Strided load, stride 2 | 100% | 50% |
| Strided load, stride 32 | 100% | 3% |
| Divergent if/else, half the wave each way | 78% | - |
| Divergent loop, lane i runs i iterations | 52% | - |
| Nested divergent loops | 34% | - |

The three strided loads (vector add does consecutive loads so count that as the first) have the same SIMD utilization. It's pretty clear that unoptimal access patterns really breaks coalescing down - serialized loads are not ideal. I don't really have a way of seeing how bad this is in terms of kernel runtime since this simulator is functional. Maybe I'll make it cycle-accurate one day LOL

# How I know it's right
I was writing out expected reg values by hand at first but oh man that got complicated fast, especially since I moved from 8 lanes to 32. It also increased the change of human error. To streamline testing I build a small scalar reference model.

There's a second model in `tests/reference.h` that runs one lane at a time with its own PC,
own registers, no active mask, no SIMT stack, no CFG, nothing GPU specific just a loop that runs all instructions. Every test runs the real wave and diffs all 32 lanes
against it, so I don't write expected values at all.

It paid off lol, especially with loops bc it caught a lot of divergence bugs that would have taken me ages to pinpoint. It also made the process of moving from 8 to 32 threads much easier!

I still kept a few hand-written tests, mainly to test the ISA and make sure its right. I don't want both, my refernce and my kernel, to be wrong. Blind leading the blind...

It all runs under ctest, and the debug build has ASan and UBSan on.

# What I'm working on
- SAXPY, transpose, and reduction kernels using my ISA
- Multiple waves, smem, barriers
- A frontend for PTX and SPIR-V (yeah... that's gonna take a while)

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
