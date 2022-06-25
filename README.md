# imp - an image processing utility
~~Graphics processing algorithms written in x86 assembly and C. Several versions are provided with an emphasis on optimizing for run-time speed. To that effect, SIMD and vectorization techniques will be heavily used and benchmarked.~~
aspiring cli bmp editing program

```console
USAGE:
    imp [-i inputfile] [-o outputfile] [-f flags] [-h]

FLAGS:
    i               invert
    g               grayscale
    d               ordered dithering
    n               uniform noise

ARGS:
    [inputfile]    24bit bmp
```

## results
![some results](/res/some-results.png)<br />
*from left to right: original, ordered dithering, inversion, uniform noise, dithering + palette quantization, several filters*

## build
```console
git clone https://github.com/feydor/imp
cd imp
mkdir build && cd build
./../build.sh
./imp -i ../david.bmp -o ../result.bmp -f igdn
```