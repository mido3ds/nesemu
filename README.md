# NESemu [WIP]
NES emulator in c++

## Requirements:

|          |           |
|----------|-----------|
| cmake    | `+3.12.1` |
| SFML     | `+2.5.0`
| Catch2   | `+2.11.1` |
|          |           |

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

