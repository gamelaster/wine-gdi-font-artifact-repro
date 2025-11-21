# Reproduction code for Wine GDI font rasterization bug

Author of this code is [vahook](https://github.com/vahook) (thank you!)
with small change made by me to use D3DX9_42.dll directly for easy reproduction.

**DISCLAIMER:** Parts of this code was written by LLM, but it's fully functional and clean/straightforward.

## Building

Code can be built by either MSVC compiler or MinGW. I used MinGW, so CMake and instructions are according to that.

```bash
mkdir out && cd out
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr/i686-w64-mingw32 -DCMAKE_INSTALL_LIBDIR:PATH=lib -DCMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES:PATH=/usr/i686-w64-mingw32/include -DCMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES:PATH=/usr/i686-w64-mingw32/include -DCMAKE_BUILD_TYPE=None -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-i686-w64-mingw32.cmake -DCMAKE_CROSSCOMPILING_EMULATOR=/usr/bin/i686-w64-mingw32-wine -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-i686.cmake
cmake --build .
cp ../D3DX9_42.dll ./
```

## Running

Just run the .exe. If the issue does not happen, configure Wine to use native version of d3dx9_42.dll.
