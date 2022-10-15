# NESemu [WIP]
NES emulator in written C++

## Requirements:

- cmake `+3.12.1`
- g++ or clang or MSVC
- ninja (optional) `+1.10.2`
- OpenAL

```sh
cmake -S. -Bbuild -GNinja
```

## Run:

```sh
cmake --build build --target nesemu -j
./build/bin/Debug/nesemu /path/to/rom.nes
```

## Test:

```sh
cmake --build build --target test
./build/bin/Debug/test
```
