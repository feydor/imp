# Graphics Processing algorithms in x86 assembly and C
Motion estimation algorithms implemented in x86-64 assembly.

## Algorithms Implemented
- Naive sum of absolute differences (SAD) in x86-64.
- Naive sum of absolute differences in C.

## Setup
'''
git clone https://github.com/feydor/motion-estimation-algorithms-x64.git
cd motion-estimation-algorithims-x64
mkdir build && cd build
meson ..
ninja
./sadx64 -i [input_file]
'''

## Todo
- Add more options for SAD; ie greatest SAD.
- Complete the benchmarking function for each algorithm.
- SIMD optimizations on x86-64 and C versions.
- Win32 executable.
- Save a bmp output file using -o.
