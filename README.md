# NESemu [WIP]
NES emulator in c++

## Requirements:

| Requirement |  Version  |
|-------------|-----------|
| cmake       | `+3.12.1` |
| SFML        | `+2.5.0`  |

On debian based distro, you can install them with:
```sh
$ sudo apt update && sudo apt install -y \
    cmake                \
    libsfml-dev          \
    libsfml-audio2.5     \
    libsfml-dev          \
    libsfml-doc          \
    libsfml-graphics2.5  \
    libsfml-network2.5   \
    libsfml-system2.5    \
    libsfml-window2.5
```

## After Clone:

Install requirements, then:

```
$ mkdir build && cd build
$ cmake ..
```

## Build:

`$ make` 

## Run:

`$ ./nesemu /path/to/rom.nes` 

## Build & Run All Tests:

`$ make test && ./test` 

## Build & Run Speicific Test:

`$ make test && ./test <name of test without file suffix>` 

