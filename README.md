# Graphics processing algorithms in x86 assembly and C
Motion estimation and other graphics processing algorithms written in x86 assembly and C. Several versions are provided with an emphasis on optimizing for run-time speed. To that effect, SIMD and vectorization techniques will be heavily used and benchmarked.

![screenshot](/res/screenshot.png)

## Algorithms Implemented
- Naive sum of absolute differences (SAD) in x86-64.
- Naive sum of absolute differences in C.

## Setup
```sh
git clone https://github.com/feydor/motion-estimation-algorithms-x64.git
cd motion-estimation-algorithims-x64
mkdir build && cd build
meson ..
ninja
./sadx64 -i [input_file]
```

## Todo
- Add more options for SAD; ie greatest SAD.
- Complete the benchmarking function for each algorithm.
- SIMD optimizations on x86-64 and C versions.
- Win32 executable.
- Save a bmp output file using -o.
