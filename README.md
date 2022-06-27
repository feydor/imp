# imp - an image processing utility
aspiring cli bmp editing program

```console
Usage: imp [OPTIONS] INPUT

Options:
    -h, --help               Display this message
    -f, --flags              Image processing passes
    -i, --input              Input filename
    -o, --output             Write output to filename

Flags:
     d     ordered dithering
     g     grayscale
     i     invert
     n     uniform noise
     p     palette quantization
```

## results
![some results](/res/some-results.png)<br />
*clockwise from top-left: original, ordered dithering, inversion, uniform noise, dithering + palette quantization, several filters*

## build
```console
git clone https://github.com/feydor/imp
cd imp
mkdir build && cd build
./../build.sh
./imp ../david.bmp -o ../result.bmp -f dg
```