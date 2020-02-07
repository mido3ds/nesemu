# NESemu [WIP]
NES emulator in c++

## Requirements:

|          |           |
|----------|-----------|
| cmake    | `+3.12.1` |
| sdl2     | `+2.0.10` |
| sdl2_ttf | `+2.0.15` |
| Catch2   | `+2.11.1` |
|          |           |

## After Clone:

Install requirements, then:

``` 
$ ./scripts/fetch-libs
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

