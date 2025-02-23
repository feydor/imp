# imp
a fun kidpix-like program for drawing and saving bitmaps, very wip

# screenshot
<img src="/res/main.png" width="600" />

## dependencies
[SDL2](https://www.libsdl.org/)<br />
[SDL2_Image](https://github.com/libsdl-org/SDL_image)<br />
[MSYS2 UCRT64](for Windows only)](https://www.msys2.org/)<br />

## build
```console
git clone https://github.com/feydor/imp
cd imp
./build.sh
./build/imp
```

### Install Windows dependencies (with MSYS2 UCRT64)
```console
pacman -S mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image
```